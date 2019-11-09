#include "Granny2Model.h"

#include "FilamentEngineInstance.h"

#include <ESOData/Filesystem/Filesystem.h>

#include <granny.h>

#include <sstream>

#include <filament/Engine.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>

#include <geometry/SurfaceOrientation.h>

#include "Granny2Renderable.h"
#include "ESOGraphicsTypes.h"
#include "DDSTexture.h"

static filament::math::float3 unpackNormal(const int16_t data[2]) {
	float v1 = (data[0] + 32768.0f) / 32768.0f;
	float v2 = (data[1] + 32768.0f) / 32768.0f;

	float iv1;
	float iv2;

	v1 = (2 * modff(v1, &iv1) - 1) * 32769.0f / 32767.0f;
	v2 = (2 * modff(v2, &iv2) - 1) * 32769.0f / 32767.0f;

	float signBit = iv1 * 2 - 1;

	float dotNormal = 1.0f - (v1 * v1 + v2 * v2);
	if (dotNormal < 0.0f)
		dotNormal = 0.0f;
	else if (dotNormal > 1.0f)
		dotNormal = 1.0f;

	float v3 = sqrtf(dotNormal) * signBit;

	return filament::math::float3(v1, v2, v3);
}

static filament::math::float3 cross(const filament::math::float3& u, const filament::math::float3& v) noexcept {
	return {
			u[1] * v[2] - u[2] * v[1],
			u[2] * v[0] - u[0] * v[2],
			u[0] * v[1] - u[1] * v[0] };
}

static float dot(const filament::math::float3& u, const filament::math::float3& v) noexcept {
	return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

Granny2Model::Granny2Model(FilamentEngineInstance* engine, uint64_t key) : m_engine(engine), m_key(key) {

}

void Granny2Model::load() {
	auto e = m_engine->engine();

	auto fileData = m_engine->fs()->readFileByKey(m_key);

	filament::TextureSampler CommonESOLikeSampler(filament::TextureSampler::MagFilter::LINEAR, filament::TextureSampler::WrapMode::REPEAT);
	CommonESOLikeSampler.setAnisotropy(7.0f);

	m_file.reset(GrannyReadEntireFileFromMemory(fileData.size(), fileData.data()));
	if (!m_file) {
		std::stringstream error;
		error << "Failed to read granny2 file " << std::hex << m_key;
		throw std::runtime_error(error.str());
	}

	auto info = GrannyGetFileInfo(m_file.get());
	
	m_vertexBuffers.reserve(info->VertexDataCount);
	for (int index = 0, count = info->VertexDataCount; index < count; index++) {
		auto data = info->VertexDatas[index];

		filament::VertexBuffer::Builder builder;
		builder.bufferCount(2);
		builder.vertexCount(data->VertexCount);

		auto vertices = std::make_unique<std::vector<ESOLikeVertex>>(data->VertexCount);
		std::vector<ESOLikeVertexAside> verticesAside(data->VertexCount);

		GrannyConvertVertexLayouts(
			data->VertexCount,
			data->VertexType,
			data->Vertices,
			ESOLikeVertexType,
			vertices->data()
		);

		GrannyConvertVertexLayouts(
			data->VertexCount,
			data->VertexType,
			data->Vertices,
			ESOLikeVertexAsideType,
			verticesAside.data()
		);

		std::vector<filament::math::float3> normals(data->VertexCount);
		std::vector<filament::math::float4> tangents(data->VertexCount);

		for (size_t index = 0, count = data->VertexCount; index < count; index++) {
			auto normal = unpackNormal(verticesAside[index].Normal);
			auto tangent = unpackNormal(verticesAside[index].Tangent);
			auto binormal = unpackNormal(verticesAside[index].Binormal);

			auto handedness = dot(binormal, cross(normal, tangent));

			normals[index] = normal;
			tangents[index] = filament::math::float4(tangent, handedness);
		}

		filament::geometry::SurfaceOrientation::Builder orientationBuilder;
		orientationBuilder.vertexCount(data->VertexCount);
		orientationBuilder.normals(normals.data(), sizeof(filament::math::float3));
		orientationBuilder.tangents(tangents.data(), sizeof(filament::math::float4));
		auto orientation = orientationBuilder.build();

		auto orientationQuats = std::make_unique<std::vector<filament::math::quath>>(data->VertexCount);
		orientation.getQuats(orientationQuats->data(), orientationQuats->size());

		builder.attribute(filament::VertexAttribute::POSITION, 0, filament::VertexBuffer::AttributeType::FLOAT3, offsetof(ESOLikeVertex, Position), sizeof(ESOLikeVertex));
		builder.attribute(filament::VertexAttribute::COLOR, 0, filament::VertexBuffer::AttributeType::UBYTE4, offsetof(ESOLikeVertex, DiffuseColor0), sizeof(ESOLikeVertex));
		builder.normalized(filament::VertexAttribute::COLOR);
		builder.attribute(filament::VertexAttribute::UV0, 0, filament::VertexBuffer::AttributeType::SHORT2, offsetof(ESOLikeVertex, TextureCoord0), sizeof(ESOLikeVertex));
		builder.normalized(filament::VertexAttribute::UV0); 
		builder.attribute(filament::VertexAttribute::UV1, 0, filament::VertexBuffer::AttributeType::SHORT2, offsetof(ESOLikeVertex, TextureCoord1), sizeof(ESOLikeVertex));
		builder.normalized(filament::VertexAttribute::UV1); 
		builder.attribute(filament::VertexAttribute::TANGENTS, 1, filament::VertexBuffer::AttributeType::HALF4);

		auto buffer = FilamentVertexBuffer(builder.build(*e), e);
		buffer->setBufferAt(
			*e,
			0,
			filament::VertexBuffer::BufferDescriptor(vertices->data(), vertices->size() * sizeof(decltype(vertices)::element_type::value_type), &bufferDescriptorReleaseCallback<ESOLikeVertex>, vertices.get())
		);
		vertices.release();

		buffer->setBufferAt(
			*e,
			1,
			filament::VertexBuffer::BufferDescriptor(orientationQuats->data(), orientationQuats->size() * sizeof(decltype(orientationQuats)::element_type::value_type), &bufferDescriptorReleaseCallback<filament::math::quath>, vertices.get())
		);
		orientationQuats.release();

		m_vertexBuffers.emplace_back(std::move(buffer));
	}

	m_indexBuffers.reserve(info->TriTopologyCount);

	for (int index = 0, count = info->TriTopologyCount; index < count; index++) {
		auto topology = info->TriTopologies[index];

		filament::IndexBuffer::Builder builder;
		if (topology->IndexCount != 0) {
			builder.bufferType(filament::IndexBuffer::IndexType::UINT);
			builder.indexCount(topology->IndexCount);
			auto buffer = FilamentIndexBuffer(builder.build(*e), e);

			buffer->setBuffer(
				*e,
				filament::IndexBuffer::BufferDescriptor(topology->Indices, topology->IndexCount * sizeof(uint32_t), releasePointerToSelf, new std::shared_ptr<Granny2Model>(shared_from_this()))
			);

			m_indexBuffers.emplace_back(std::move(buffer));
		}
		else {
			builder.bufferType(filament::IndexBuffer::IndexType::USHORT);
			builder.indexCount(topology->Index16Count);
			auto buffer = FilamentIndexBuffer(builder.build(*e), e);

			buffer->setBuffer(
				*e,
				filament::IndexBuffer::BufferDescriptor(topology->Indices16, topology->Index16Count * sizeof(uint16_t), releasePointerToSelf, new std::shared_ptr<Granny2Model>(shared_from_this()))
			); 
			
			m_indexBuffers.emplace_back(std::move(buffer));
		}
	}

	m_materials.reserve(info->MaterialCount);

	for (int index = 0, count = info->MaterialCount; index < count; index++) {
		auto material = info->Materials[index];

		if (material->MapCount != 0) {
			auto instance = FilamentMaterialInstance(m_engine->esoLikeMaterial()->createInstance(), e);
			
			ESOMaterial parameters;
			GrannyConvertSingleObject(
				material->ExtendedData.Type,
				material->ExtendedData.Object,
				ESOMaterialType,
				&parameters,
				nullptr
			);

			uint64_t key;
			bool hasDiffuse = findMap(material, "diffuse", key);

			instance->setParameter("diffuseTexturePresent", hasDiffuse);
			if (hasDiffuse) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("diffuseTexture", texture->texture(), CommonESOLikeSampler);
			}

			bool hasNormal = findMap(material, "normal", key);

			instance->setParameter("normalTexturePresent", hasNormal);
			if (hasNormal) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("normalTexture", texture->texture(), CommonESOLikeSampler);
			}

			bool hasSpecular = findMap(material, "specular", key);

			instance->setParameter("specularTexturePresent", hasNormal);
			if (hasSpecular) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("specularTexture", texture->texture(), CommonESOLikeSampler);
			}

			m_materials.emplace_back(std::move(instance));

		} else {
			m_materials.emplace_back();
		}
	}
}

Granny2Model::~Granny2Model() = default;

bool Granny2Model::findMap(granny_material* material, const char* map, uint64_t& key) {
	for (int index = 0, count = material->MapCount; index < count; index++) {
		auto& mapDef = material->Maps[index];

		if (strcmp(mapDef.Usage, map) == 0) {
			auto &texture = mapDef.Material->Texture->ExtendedData;

			ESOTexture info;
			GrannyConvertSingleObject(
				texture.Type,
				texture.Object,
				ESOTextureType,
				&info,
				nullptr
			);

			key = info.fileIndex;

			return true;
		}
	}

	return false;
}

template<typename T>
void Granny2Model::bufferDescriptorReleaseCallback(void* buffer, size_t size, void* user) {
	(void)buffer;
	(void)size;

	delete static_cast<std::vector<T>*>(user);
}

void Granny2Model::releasePointerToSelf(void* buffer, size_t size, void* user) {
	(void)buffer;
	(void)size;

	delete static_cast<std::shared_ptr<Granny2Model> *>(user);
}

std::unique_ptr<Granny2Renderable> Granny2Model::createInstance() {
	auto info = GrannyGetFileInfo(m_file.get());
	if (info->ModelCount == 0)
		return nullptr;

	if (info->ModelCount > 1)
		throw std::logic_error("one model per gr2 is expected");

	auto e = m_engine->engine();

	auto renderable = std::make_unique<Granny2Renderable>(shared_from_this());

	auto model = info->Models[0];

	int totalGroups = 0;
	for (int binding = 0, bindings = model->MeshBindingCount; binding < bindings; binding++) {
		totalGroups += model->MeshBindings[binding].Mesh->PrimaryTopology->GroupCount;
	}

	filament::RenderableManager::Builder builder(totalGroups);
	int globalGroupIndex = 0;

	filament::Box boundingBox;

	for (int binding = 0, bindings = model->MeshBindingCount; binding < bindings; binding++) {
		auto mesh = model->MeshBindings[binding].Mesh;
		
		for (int groupIndex = 0, groups = mesh->PrimaryTopology->GroupCount; groupIndex < groups; groupIndex++) {
			auto& group = mesh->PrimaryTopology->Groups[groupIndex];

			builder.geometry(
				globalGroupIndex,
				filament::RenderableManager::PrimitiveType::TRIANGLES,
				findVertexBuffer(info, mesh->PrimaryVertexData),
				findIndexBuffer(info, mesh->PrimaryTopology),
				group.TriFirst * 3,
				group.TriCount * 3
			);
			builder.material(globalGroupIndex, findMaterial(info, mesh->MaterialBindings[group.MaterialIndex].Material));
			globalGroupIndex++;
		}

		ESOMeshExtendedData meshExtended;
		GrannyConvertSingleObject(
			mesh->ExtendedData.Type,
			mesh->ExtendedData.Object,
			ESOMeshExtendedDataType,
			&meshExtended,
			nullptr
		);

		boundingBox.unionSelf(filament::Box().set(
			filament::math::float3(meshExtended.BBoxMin[0], meshExtended.BBoxMin[1], meshExtended.BBoxMin[2]),
			filament::math::float3(meshExtended.BBoxMax[0], meshExtended.BBoxMax[1], meshExtended.BBoxMax[2])
		));
	}
	
	builder.boundingBox(boundingBox);
	builder.culling(false);
	builder.build(*e, renderable->entity);


	return renderable;
}

filament::VertexBuffer* Granny2Model::findVertexBuffer(granny_file_info* info, granny_vertex_data* vertexData) {
	for (int index = 0, count = info->VertexDataCount; index < count; index++) {
		if (info->VertexDatas[index] == vertexData)
			return m_vertexBuffers[index].get();
	}
	throw std::logic_error("vertex buffer not found");
}

filament::IndexBuffer* Granny2Model::findIndexBuffer(granny_file_info* info, granny_tri_topology* triangleTopology) {
	for (int index = 0, count = info->TriTopologyCount; index < count; index++) {
		if (info->TriTopologies[index] == triangleTopology)
			return m_indexBuffers[index].get();
	}
	throw std::logic_error("index buffer not found");
}

filament::MaterialInstance* Granny2Model::findMaterial(granny_file_info* info, granny_material* material) {
	for (int index = 0, count = info->MaterialCount; index < count; index++) {
		if (info->Materials[index] == material)
			return m_materials[index].get();
	}
	throw std::logic_error("material not found");
}

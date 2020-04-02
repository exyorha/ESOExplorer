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
#include <ESOData/Granny2/ESOGraphicsTypes.h>
#include "DDSTexture.h"

struct AuxVertex {
	filament::math::quath orientation;
	uint32_t dummyColor;
};

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

static void unpackAttribute(std::vector<filament::math::float3>& dest, const void* data, size_t count, size_t stride, filament::VertexBuffer::AttributeType type, bool normalized) {
	for (size_t index = 0; index < count; index++) {
		filament::math::float3 result;

		if (type == filament::VertexBuffer::AttributeType::FLOAT3) {
			auto vector = static_cast<const float*>(data);

			result = filament::math::float3(vector[0], vector[1], vector[2]);

		} else if (type == filament::VertexBuffer::AttributeType::SHORT2 && normalized) {
			result = unpackNormal(static_cast<const int16_t*>(data));
		} else {
			throw std::logic_error("can't unpack type " + std::to_string(static_cast<int>(type)));
		}

		dest[index] = result;
		data = reinterpret_cast<const uint8_t*>(data) + stride;
	}
}

static void translateUVSet(
	void* data,
	size_t vertexCount,
	size_t stride,
	filament::VertexBuffer::AttributeType& attributeType
) {
	if (attributeType == filament::VertexBuffer::AttributeType::FLOAT2)
		return;

	for (size_t index = 0; index < vertexCount; index++) {
		if (attributeType == filament::VertexBuffer::AttributeType::SHORT2) {
			auto values = static_cast<int16_t*>(data);
			float u = values[0] / 256.0f;
			float v = values[1] / 256.0f;

			auto dest = reinterpret_cast<uint16_t*>(values);
			dest[0] = GrannyReal32ToReal16(u);
			dest[1] = GrannyReal32ToReal16(v);
		}
		else {
			throw std::runtime_error("cannot translate UV set, type " + std::to_string(static_cast<int>(attributeType)));
		}

		data = reinterpret_cast<uint8_t*>(data) + stride;
	}

	attributeType = filament::VertexBuffer::AttributeType::HALF2;
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

		filament::math::float2 uvScales(1.0f, 1.0f);

		std::vector<filament::math::float3> normals(data->VertexCount);
		std::vector<filament::math::float3> tangents(data->VertexCount);
		std::vector<filament::math::float3> binormals(data->VertexCount);

		size_t vertexSize = 0;
		for (auto type = data->VertexType; type->Type != GrannyEndMember; type++) {
			vertexSize += GrannyGetMemberTypeSize(type);
		}

		printf("Vertex size: %zu\n", vertexSize);
		
		size_t offset = 0;
		unsigned int attributesPresent = 0;

		for (auto type = data->VertexType; type->Type != GrannyEndMember; type++) {
			filament::VertexBuffer::AttributeType attributeType;
			bool normalized = false;

			switch (type->Type) {
			case GrannyReal32Member:
				switch (GrannyGetMemberArrayWidth(type)) {
				case 1:
					attributeType = filament::VertexBuffer::AttributeType::FLOAT;
					break;

				case 2:
					attributeType = filament::VertexBuffer::AttributeType::FLOAT2;
					break;

				case 3:
					attributeType = filament::VertexBuffer::AttributeType::FLOAT3;
					break;

				case 4:
					attributeType = filament::VertexBuffer::AttributeType::FLOAT4;
					break;

				default:
					throw std::logic_error("unsupported array width in vertex");
				}
				break;

			case GrannyUInt8Member:
			case GrannyNormalUInt8Member:
				normalized = true;
				switch (GrannyGetMemberArrayWidth(type)) {
				case 1:
					attributeType = filament::VertexBuffer::AttributeType::UBYTE;
					break;

				case 2:
					attributeType = filament::VertexBuffer::AttributeType::UBYTE2;
					break;

				case 3:
					attributeType = filament::VertexBuffer::AttributeType::UBYTE3;
					break;

				case 4:
					attributeType = filament::VertexBuffer::AttributeType::UBYTE4;
					break;

				default:
					throw std::logic_error("unsupported array width in vertex");
				}
				break;

			case GrannyReal16Member:
				normalized = true;
				switch (GrannyGetMemberArrayWidth(type)) {
				case 1:
					attributeType = filament::VertexBuffer::AttributeType::SHORT;
					break;

				case 2:
					attributeType = filament::VertexBuffer::AttributeType::SHORT2;
					break;

				case 3:
					attributeType = filament::VertexBuffer::AttributeType::SHORT3;
					break;

				case 4:
					attributeType = filament::VertexBuffer::AttributeType::SHORT4;
					break;

				default:
					throw std::logic_error("unsupported array width in vertex");
				}
				break;

			default:
				throw std::logic_error("unsupported type " + std::to_string(static_cast<int>(type->Type)) + " in vertex");
				break;
			}

			printf("attribute %s at offset %zu, type %d, normalized: %d\n", type->Name, offset, attributeType, normalized);

			if (strcmp(type->Name, "Position") == 0) {
				builder.attribute(filament::VertexAttribute::POSITION, 0, attributeType, offset, vertexSize);
				builder.normalized(filament::VertexAttribute::POSITION, normalized);

				if (data->VertexCount > 0) {
					std::vector<filament::math::float3> positions(data->VertexCount);
					unpackAttribute(positions, static_cast<uint8_t*>(data->Vertices) + offset, data->VertexCount, vertexSize, attributeType, normalized);

					filament::math::float3 min = positions[0], max = positions[0];

					for (auto& vertex : positions) {
						min[0] = std::min(vertex[0], min[0]);
						min[1] = std::min(vertex[1], min[1]);
						min[2] = std::min(vertex[2], min[2]);
						max[0] = std::max(vertex[0], max[0]);
						max[1] = std::max(vertex[1], max[1]);
						max[2] = std::max(vertex[2], max[2]);
					}

					m_backupBoundingBoxes.emplace_back(
						filament::Box().set(min, max)
					);
				}
				else {

					m_backupBoundingBoxes.emplace_back();
				}
			}
			else if (strcmp(type->Name, "DiffuseColor0") == 0) {
				builder.attribute(filament::VertexAttribute::COLOR, 0, attributeType, offset, vertexSize);
				builder.normalized(filament::VertexAttribute::COLOR, normalized);
				attributesPresent |= 1 << filament::VertexAttribute::COLOR;
			}
			else if (strcmp(type->Name, "Normal") == 0) {
				unpackAttribute(normals, static_cast<uint8_t*>(data->Vertices) + offset, data->VertexCount, vertexSize, attributeType, normalized);
			}
			else if (strcmp(type->Name, "Tangent") == 0) {
				unpackAttribute(tangents, static_cast<uint8_t*>(data->Vertices) + offset, data->VertexCount, vertexSize, attributeType, normalized);
			}
			else if (strcmp(type->Name, "Binormal") == 0) {
				unpackAttribute(binormals, static_cast<uint8_t*>(data->Vertices) + offset, data->VertexCount, vertexSize, attributeType, normalized);
			}
			else if (strcmp(type->Name, "TextureCoord0") == 0 || strcmp(type->Name, "TextureCoordinates0") == 0) {
				translateUVSet(static_cast<uint8_t*>(data->Vertices) + offset, data->VertexCount, vertexSize, attributeType);

				builder.attribute(filament::VertexAttribute::UV0, 0, attributeType, offset, vertexSize);
				builder.normalized(filament::VertexAttribute::UV0, normalized);
				attributesPresent |= 1 << filament::VertexAttribute::UV0;
			}
			else if (strcmp(type->Name, "TextureCoord1") == 0 || strcmp(type->Name, "TextureCoordinates1") == 0) {
				translateUVSet(static_cast<uint8_t*>(data->Vertices) + offset, data->VertexCount, vertexSize, attributeType);

				builder.attribute(filament::VertexAttribute::UV1, 0, attributeType, offset, vertexSize);
				builder.normalized(filament::VertexAttribute::UV1, normalized);
				attributesPresent |= 1 << filament::VertexAttribute::UV1;
			}
			else if (strcmp(type->Name, "BoneWeights") == 0) {
				builder.attribute(filament::VertexAttribute::BONE_WEIGHTS, 0, attributeType, offset, vertexSize);
				builder.normalized(filament::VertexAttribute::BONE_WEIGHTS, true);
			}
			else if (strcmp(type->Name, "BoneIndices") == 0) {
				builder.attribute(filament::VertexAttribute::BONE_INDICES, 0, attributeType, offset, vertexSize);
			}

			offset += GrannyGetMemberTypeSize(type);
		}

		std::vector<filament::math::float4> packedTangents(data->VertexCount);
		
		for (size_t index = 0, count = data->VertexCount; index < count; index++) {
			const auto &normal = normals[index];
			const auto &tangent = tangents[index];
			const auto &binormal = binormals[index];

			auto handedness = dot(binormal, cross(normal, tangent));

			packedTangents[index] = filament::math::float4(tangent, handedness);
		}

		filament::geometry::SurfaceOrientation::Builder orientationBuilder;
		orientationBuilder.vertexCount(data->VertexCount);
		orientationBuilder.normals(normals.data(), sizeof(filament::math::float3));
		orientationBuilder.tangents(packedTangents.data(), sizeof(filament::math::float4));
		auto orientation = orientationBuilder.build();

		auto orientationQuats = std::make_unique<std::vector<AuxVertex>>(data->VertexCount);

		for (auto& aux : *orientationQuats) {
			aux.dummyColor = 0xFFFFFF;
		}

		orientation.getQuats(&orientationQuats->data()->orientation, orientationQuats->size(), sizeof(AuxVertex));

		builder.attribute(filament::VertexAttribute::TANGENTS, 1, filament::VertexBuffer::AttributeType::HALF4, offsetof(AuxVertex, orientation), sizeof(AuxVertex));
		
		for (auto attributeToDummyOut : { filament::VertexAttribute::COLOR, filament::VertexAttribute::UV0, filament::VertexAttribute::UV1 }) {
			if (!(attributesPresent & (1 << attributeToDummyOut))) {
				builder.attribute(attributeToDummyOut, 1, filament::VertexBuffer::AttributeType::UBYTE4, offsetof(AuxVertex, dummyColor), sizeof(AuxVertex));
				builder.normalized(attributeToDummyOut, true);
			}
		}

		auto buffer = FilamentVertexBuffer(builder.build(*e), e);
		buffer->setBufferAt(
			*e,
			0,
			filament::VertexBuffer::BufferDescriptor(
				data->Vertices,
				data->VertexCount * vertexSize,
				&releasePointerToSelf, new std::shared_ptr<Granny2Model>(shared_from_this())
			)
		);
	
		buffer->setBufferAt(
			*e,
			1,
			filament::VertexBuffer::BufferDescriptor(
				orientationQuats->data(),
				orientationQuats->size() * sizeof(AuxVertex),
				&bufferDescriptorReleaseCallback<AuxVertex>,
				orientationQuats.get())
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

		if (material->Texture == nullptr) {
			esodata::ESOMaterial parameters;
			GrannyConvertSingleObject(
				material->ExtendedData.Type,
				material->ExtendedData.Object,
				esodata::ESOMaterialType,
				&parameters,
				nullptr
			);

			filament::Material* baseMaterial;

			// 1 - no alpha
			// 3 - masking
			switch (parameters.alpha) {
			case 1:
				baseMaterial = m_engine->esoLikeMaterialOpaque();
				break;

			case 2:
				baseMaterial = m_engine->esoLikeMaterialTransparent();
				break;

			case 3:
			case 4:
				baseMaterial = m_engine->esoLikeMaterialMasked();
				break;

			default:
				throw std::logic_error("unsupported alpha mode " + std::to_string(parameters.alpha));
			}

			auto instance = FilamentMaterialInstance(baseMaterial->createInstance(), e);

			uint64_t key;
			bool hasDiffuse = findMap(material, "diffuse", key) && key != 0;

			instance->setParameter("diffuseTexturePresent", hasDiffuse);
			if (hasDiffuse) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("diffuseTexture", texture->texture(), CommonESOLikeSampler);
			}

			bool hasNormal = findMap(material, "normal", key) && key != 0;

			instance->setParameter("normalTexturePresent", hasNormal);
			if (hasNormal) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("normalTexture", texture->texture(), CommonESOLikeSampler);
			}

			bool hasDetail = findMap(material, "detail", key) && key != 0;

			instance->setParameter("detailTexturePresent", hasDetail);
			if (hasDetail) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("detailTexture", texture->texture(), CommonESOLikeSampler);
			}
			bool hasDiffuse2 = findMap(material, "diffuse2", key) && key != 0;

			instance->setParameter("diffuse2TexturePresent", hasDiffuse2);
			if (hasDiffuse2) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("diffuse2Texture", texture->texture(), CommonESOLikeSampler);
			}

			bool hasSpecular = findMap(material, "specular", key) && key != 0;

			instance->setParameter("specularTexturePresent", hasSpecular);
			if (hasSpecular) {
				auto texture = m_engine->loadTexture(key);
				m_requiredTextures.emplace(texture);
				instance->setParameter("specularTexture", texture->texture(), CommonESOLikeSampler);
			}


			instance->setParameter("fresnel", parameters.bFresnel != 0);
			instance->setParameter("glow", parameters.bGlowEnable != 0);
			instance->setParameter("g_detailValues", filament::math::float4(
				parameters.ZosGlossiness2,
				parameters.ZosFresnel2,
				parameters.ZosGlow2,
				0.0f
			));
			instance->setParameter("g_glowFactor", parameters.ZosDayGlow);

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

			esodata::ESOTexture info;
			GrannyConvertSingleObject(
				texture.Type,
				texture.Object,
				esodata::ESOTextureType,
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
		
		filament::Box backupBox;

		auto vertexBuffer = findVertexBuffer(info, mesh->PrimaryVertexData, backupBox);
		auto indexBuffer = findIndexBuffer(info, mesh->PrimaryTopology);

		for (int groupIndex = 0, groups = mesh->PrimaryTopology->GroupCount; groupIndex < groups; groupIndex++) {
			auto& group = mesh->PrimaryTopology->Groups[groupIndex];
			
			builder.geometry(
				globalGroupIndex,
				filament::RenderableManager::PrimitiveType::TRIANGLES,
				vertexBuffer,
				indexBuffer,
				group.TriFirst * 3,
				group.TriCount * 3
			);
			builder.material(globalGroupIndex, findMaterial(info, mesh->MaterialBindings[group.MaterialIndex].Material));
			globalGroupIndex++;
		}

		auto meshBox = filament::Box();
		if (mesh->ExtendedData.Object) {
			esodata::ESOMeshExtendedData meshExtended;
			GrannyConvertSingleObject(
				mesh->ExtendedData.Type,
				mesh->ExtendedData.Object,
				esodata::ESOMeshExtendedDataType,
				&meshExtended,
				nullptr
			);

			meshBox.set(
				filament::math::float3(meshExtended.BBoxMin[0], meshExtended.BBoxMin[1], meshExtended.BBoxMin[2]),
				filament::math::float3(meshExtended.BBoxMax[0], meshExtended.BBoxMax[1], meshExtended.BBoxMax[2])
			);
		}

		if (meshBox.isEmpty()) {
			meshBox = backupBox;
		}

		boundingBox.unionSelf(meshBox);
	}
	
	builder.boundingBox(boundingBox);
	builder.culling(false);
	builder.build(*e, renderable->entity);


	return renderable;
}

filament::VertexBuffer* Granny2Model::findVertexBuffer(granny_file_info* info, granny_vertex_data* vertexData, filament::Box& backupBoundingBox) {
	for (int index = 0, count = info->VertexDataCount; index < count; index++) {
		if (info->VertexDatas[index] == vertexData) {
			backupBoundingBox = m_backupBoundingBoxes[index];
			return m_vertexBuffers[index].get();
		}
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

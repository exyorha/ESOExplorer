#include "DDSTexture.h"

#include "FilamentEngineInstance.h"

#include <ESOData/Filesystem/Filesystem.h>

#include <DirectXTex.h>

#include <sstream>

#include <filament/Texture.h>
#include <filament/Engine.h>

DDSTexture::DDSTexture(FilamentEngineInstance* engine, uint64_t key) : m_engine(engine), m_key(key) {

}

DDSTexture::~DDSTexture() = default;

void DDSTexture::load() {
	auto e = m_engine->engine();

	auto fileData = m_engine->fs()->readFileByKey(m_key);

	auto image = std::make_shared<DirectX::ScratchImage>();

	auto hr = DirectX::LoadFromDDSMemory(fileData.data(), fileData.size(), DirectX::DDS_FLAGS_NONE, nullptr, *image);
	if (FAILED(hr)) {
		std::stringstream error;
		error << "Failed to read DDS file " << std::hex << m_key;
		throw std::runtime_error(error.str());
	}

	auto meta = image->GetMetadata();

	filament::Texture::Builder builder;
	builder.width(meta.width);
	
	if (meta.dimension >= DirectX::TEX_DIMENSION_TEXTURE2D)
		builder.height(meta.height);

	if (meta.dimension >= DirectX::TEX_DIMENSION_TEXTURE3D)
		builder.depth(meta.depth);

	builder.levels(meta.mipLevels);

	if (meta.miscFlags & DirectX::TEX_MISC_TEXTURECUBE)
		builder.sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP);
	else
		builder.sampler(filament::Texture::Sampler::SAMPLER_2D);
	
	filament::backend::CompressedPixelDataType type;

	switch (meta.format) {
	case DXGI_FORMAT_BC1_UNORM:
		builder.format(filament::Texture::InternalFormat::DXT1_RGBA);
		type = filament::backend::CompressedPixelDataType::DXT1_RGBA;
		break;

	case DXGI_FORMAT_BC3_UNORM:
		builder.format(filament::Texture::InternalFormat::DXT5_RGBA);
		type = filament::backend::CompressedPixelDataType::DXT5_RGBA;
		break;

	default:
		std::stringstream error;
		error << "Implement DXGI format " << meta.format;
		throw std::runtime_error(error.str());
	}

	m_texture = FilamentTexture(builder.build(*e), e);

	for (size_t level = 0; level < meta.mipLevels; level++) {
		if (meta.miscFlags & DirectX::TEX_MISC_TEXTURECUBE) {
			__debugbreak();
		}
		else {
			auto img = image->GetImage(level, 0, 0);

			m_texture->setImage(
				*e,
				level,
				filament::Texture::PixelBufferDescriptor(
					img->pixels,
					img->slicePitch,
					type,
					img->slicePitch,
					releaseImage,
					new std::shared_ptr<DirectX::ScratchImage>(image)
				)
			);
		}
	}
}

void DDSTexture::releaseImage(void* buffer, size_t size, void* user) {
	(void)buffer;
	(void)size;
	delete static_cast<std::shared_ptr<DirectX::ScratchImage>*>(user);
}


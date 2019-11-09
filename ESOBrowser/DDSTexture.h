#ifndef DDS_TEXTURE_H
#define DDS_TEXTURE_H

#include <memory>

#include "FilamentTypeHelpers.h"

class FilamentEngineInstance;

class DDSTexture final : public std::enable_shared_from_this<DDSTexture> {
public:
	DDSTexture(FilamentEngineInstance* engine, uint64_t key);
	~DDSTexture();
	
	DDSTexture(const DDSTexture& other) = delete;
	DDSTexture &operator =(const DDSTexture& other) = delete;

	void load();

	inline filament::Texture* texture() {
		return m_texture.get();
	}

private:
	static void releaseImage(void* buffer, size_t size, void* user);

	FilamentEngineInstance* m_engine;
	uint64_t m_key;
	FilamentTexture m_texture;
};

#endif

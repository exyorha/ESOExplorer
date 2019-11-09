#ifndef FILAMENT_ENGINE_INSTANCE_H
#define FILAMENT_ENGINE_INSTANCE_H

#include "FilamentTypeHelpers.h"

#include <unordered_map>

namespace esodata {
	class Filesystem;
}

class Granny2Model;
class DDSTexture;

class QString;

class FilamentEngineInstance {
public:
	explicit FilamentEngineInstance(const esodata::Filesystem *fs);
	~FilamentEngineInstance();

	FilamentEngineInstance(const FilamentEngineInstance& other) = delete;
	FilamentEngineInstance &operator =(const FilamentEngineInstance& other) = delete;

	inline const esodata::Filesystem* fs() {
		return m_fs;
	}

	inline filament::Engine* engine() {
		return m_engine.get();
	}

	inline filament::Material* esoLikeMaterial() {
		return m_esoLikeMaterial.get();
	}

	std::shared_ptr<Granny2Model> loadModel(uint64_t key);
	std::shared_ptr<DDSTexture> loadTexture(uint64_t key);

private:
	FilamentMaterial loadMaterial(const QString& name);

	const esodata::Filesystem* m_fs;
	FilamentEngine m_engine;
	std::unordered_map<uint64_t, std::weak_ptr<Granny2Model>> m_models;
	std::unordered_map<uint64_t, std::weak_ptr<DDSTexture>> m_textures;
	FilamentMaterial m_esoLikeMaterial;
};

#endif

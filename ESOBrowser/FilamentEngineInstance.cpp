#include "FilamentEngineInstance.h"
#include "Granny2Model.h"
#include "DDSTexture.h"

#include <filament/Engine.h>
#include <filament/Material.h>

#include <QCoreApplication>

#include <fstream>

FilamentEngineInstance::FilamentEngineInstance(const esodata::Filesystem* fs) : m_fs(fs), m_engine(filament::Engine::create()),
	m_esoLikeMaterial(loadMaterial(QStringLiteral("ESOLikeMaterial.matc"))) {

}

FilamentEngineInstance::~FilamentEngineInstance() = default;

FilamentMaterial FilamentEngineInstance::loadMaterial(const QString& name) {
	std::ifstream stream;
	stream.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
	stream.open((QCoreApplication::applicationDirPath() + "/" + name).toStdWString(), std::ios::in | std::ios::binary);
	stream.seekg(0, std::ios::end);
	auto length = static_cast<size_t>(stream.tellg());
	stream.seekg(0);

	std::vector<unsigned char> data(length);
	stream.read(reinterpret_cast<char*>(data.data()), data.size());

	filament::Material::Builder builder;
	builder.package(data.data(), data.size());
	return FilamentMaterial(builder.build(*m_engine), m_engine.get());
}

std::shared_ptr<Granny2Model> FilamentEngineInstance::loadModel(uint64_t key) {
	auto it = m_models.find(key);
	if (it == m_models.end()) {
		auto model = std::make_shared<Granny2Model>(this, key);
		model->load();
		m_models.emplace(key, model);
		return model;
	}

	auto model = it->second.lock();
	if (model)
		return model;

	model = std::make_shared<Granny2Model>(this, key);
	model->load();
	it->second = model;
	return model;
}

std::shared_ptr<DDSTexture> FilamentEngineInstance::loadTexture(uint64_t key) {
	auto it = m_textures.find(key);
	if (it == m_textures.end()) {
		auto texture = std::make_shared<DDSTexture>(this, key);
		texture->load();
		m_textures.emplace(key, texture);
		return texture;
	}

	auto texture = it->second.lock();
	if (texture)
		return texture;

	texture = std::make_shared<DDSTexture>(this, key);
	texture->load();
	it->second = texture;
	return texture;
}

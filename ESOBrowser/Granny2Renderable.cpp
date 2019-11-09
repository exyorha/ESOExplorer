#include "Granny2Renderable.h"
#include "Granny2Model.h"
#include "FilamentEngineInstance.h"

#include <utils/EntityManager.h>
#include <filament/Engine.h>
#include <filament/TransformManager.h>
#include <filament/RenderableManager.h>

Granny2Renderable::Granny2Renderable(const std::shared_ptr<Granny2Model>& model) : m_model(model) {
	auto e = m_model->engine()->engine();

	entity = utils::EntityManager::get().create();
	e->getTransformManager().create(entity);
}

Granny2Renderable::~Granny2Renderable() {
	auto e = m_model->engine()->engine();
	e->getRenderableManager().destroy(entity);

	e->getTransformManager().destroy(entity);
	utils::EntityManager::get().destroy(entity);
}

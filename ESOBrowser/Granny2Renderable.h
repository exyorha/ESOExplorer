#ifndef GRANNY2_RENDERABLE_H
#define GRANNY2_RENDERABLE_H

#include <memory>

#include <utils/Entity.h>

class Granny2Model;

class Granny2Renderable {
public:
	explicit Granny2Renderable(const std::shared_ptr<Granny2Model>& model);
	~Granny2Renderable();

	Granny2Renderable(const Granny2Renderable& other) = delete;
	Granny2Renderable &operator =(const Granny2Renderable& other) = delete;

	utils::Entity entity;

private:
	std::shared_ptr<Granny2Model> m_model;
};

#endif

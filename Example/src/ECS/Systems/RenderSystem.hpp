#pragma once

#include "../../pch.hpp"

class RenderSystem : public ECS::System
{
protected:
	Render::Renderer* renderer;

public:
	RenderSystem(Render::Renderer* _renderer) : renderer(_renderer) {}

	virtual void Update(ECS::ComponentManager& _componentManager, const float& _dt);
};
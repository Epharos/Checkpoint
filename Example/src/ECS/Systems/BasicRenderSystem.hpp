#pragma once

#include "RenderSystem.hpp"

class BasicRenderSystem : public RenderSystem<BasicRenderer>
{
protected:
	QueryVector query;

	Entity directionalLightEntity = ECS::EntityManager::NULL_ENTITY;

public:
	BasicRenderSystem(BasicRenderer* _renderer);

	void OnRegister(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager) override;
	void Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt) override;
	void Cleanup() override;
};
#pragma once

#include "RenderSystem.hpp"

class BasicRenderSystem : public RenderSystem<BasicRenderer>
{
protected:
	QueryVector query;

	Entity directionalLightEntity = ECS::EntityManager::NULL_ENTITY;
	
	float* cascadeSplits;
	glm::vec3* frustumCorners;
	glm::mat4* lightViewProjections;

	std::vector<Render::InstanceGroup> PrepareInstanceGroups(ECS::ComponentManager& _componentManager, QueryVector query);
	float* GetCascadeSplits(const float& _near, const float& _far, const uint8_t& _cascadeCount, const float& _lambda);
	glm::vec3* GetFrustumCorners(const glm::mat4& _viewProjection);
	glm::mat4* GetCascadeProjections(const glm::mat4 _cameraProj, const glm::mat4& _cameraView, const glm::mat4& _cameraViewproj, const uint32_t& _cascadeCount, const float& _near, const float& _far, const glm::vec3& _lightDir);

public:
	BasicRenderSystem(BasicRenderer* _renderer);

	void OnRegister(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager) override;
	void Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt) override;
	void Cleanup() override;
};
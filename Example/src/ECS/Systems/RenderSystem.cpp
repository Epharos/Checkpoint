#include "pch.hpp"

#include "RenderSystem.hpp"
std::vector<Render::InstanceGroup> PrepareInstanceGroups(ECS::ComponentManager& _componentManager);

void RenderSystem::Update(ECS::ComponentManager& _componentManager, const float& _dt)
{
	auto instanceGroups = PrepareInstanceGroups(_componentManager);

	renderer->Render(instanceGroups);
}

std::vector<Render::InstanceGroup> PrepareInstanceGroups(ECS::ComponentManager& _componentManager)
{
	std::vector<Render::InstanceGroup> instanceGroups;
	std::unordered_map<Resource::Mesh*, std::vector<Render::TransformData>> meshTransforms;

	_componentManager.ForEachComponent<MeshRenderer>([&](Entity _entity, MeshRenderer& _meshRenderer)
		{
			auto transform = _componentManager.GetComponent<Transform>(_entity);

			glm::mat4 modelMatrix = transform.GetModelMatrix();
			glm::mat4 normalMatrix = glm::mat4(transform.GetNormalMatrix());

			meshTransforms[_meshRenderer.mesh].push_back(Render::TransformData(modelMatrix, normalMatrix));
		});

	for (auto& [mesh, transforms] : meshTransforms)
	{
		instanceGroups.push_back({ mesh, transforms });
	}

	return instanceGroups;
}
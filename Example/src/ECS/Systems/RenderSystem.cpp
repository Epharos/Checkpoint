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

	std::unordered_map<std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>, 
		std::vector<Render::TransformData>, 
		Helper::Hash::TupleHash<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>> data;

	_componentManager.ForEachComponent<MeshRenderer>([&](Entity _entity, MeshRenderer& _meshRenderer)
		{
			auto transform = _componentManager.GetComponent<Transform>(_entity);

			//LOG_DEBUG(MF("Entity: ", _entity.id, " Material: ", _meshRenderer.materialInstance));

			glm::mat4 modelMatrix = transform.GetModelMatrix();
			glm::mat4 normalMatrix = glm::mat4(transform.GetNormalMatrix());

			data[std::make_tuple(_meshRenderer.materialInstance->GetMaterial(), _meshRenderer.mesh, _meshRenderer.materialInstance)].push_back({modelMatrix, normalMatrix});
		});

	uint32_t instanceOffset = 0;

	for (auto& [tuple, tdata] : data)
	{
		auto [material, mesh, materialInstance] = tuple;

		instanceGroups.push_back({ material, materialInstance, mesh, tdata, instanceOffset });

		//LOG_DEBUG(MF("Material: ", material, " Mesh: ", mesh, " MaterialInstance: ", materialInstance, " InstanceOffset: ", instanceOffset));

		instanceOffset += tdata.size();
	}

	std::sort(instanceGroups.begin(), instanceGroups.end(), [](const Render::InstanceGroup& a, const Render::InstanceGroup& b)
		{
			return a.material < b.material && a.mesh < b.mesh && a.materialInstance < b.materialInstance;
		});

	return instanceGroups;
}
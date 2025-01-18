#include "pch.hpp"

#include "RenderSystem.hpp"



std::vector<Render::InstanceGroup> PrepareInstanceGroups(ECS::ComponentManager& _componentManager, QueryVector query);

void RenderSystem::Update(ECS::ComponentManager& _componentManager, const float& _dt)
{
	if(query.empty())
	{
		for (auto [mesh, transform] : _componentManager.QueryArchetype<MeshRenderer, Transform>())
		{
			//std::tuple<MeshRenderer&, Transform&> tuple = std::make_tuple<MeshRenderer&, Transform&>(mesh, transform);
			query.emplace_back(mesh, transform);
		}
	}

	auto instanceGroups = PrepareInstanceGroups(_componentManager, query);

	renderer->Render(instanceGroups);
}

std::vector<Render::InstanceGroup> PrepareInstanceGroups(ECS::ComponentManager& _componentManager, QueryVector query)
{
	std::vector<Render::InstanceGroup> instanceGroups;

	std::unordered_map<std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>, 
		std::vector<Render::TransformData>, 
		Helper::Hash::TupleHash<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>> data;

	for (auto [mesh, transform] : query)
	{
		glm::mat4 modelMatrix = transform.GetModelMatrix();
		glm::mat4 normalMatrix = glm::mat4(transform.GetNormalMatrix());

		data[std::make_tuple(mesh.materialInstance->GetMaterial(), mesh.mesh, mesh.materialInstance)].push_back({ modelMatrix, normalMatrix });
	}

	// Try to optimize it by storing the pair<MeshRenderer, Transform> and update it only when needed (when an entity gets the MeshRenderer or Transform component)

	/*_componentManager.ForEachArchetype<MeshRenderer, Transform>([&](Entity _entity, MeshRenderer& _meshRenderer, Transform& _transform)
		{
			glm::mat4 modelMatrix = _transform.GetModelMatrix();
			glm::mat4 normalMatrix = glm::mat4(_transform.GetNormalMatrix());

			data[std::make_tuple(_meshRenderer.materialInstance->GetMaterial(), _meshRenderer.mesh, _meshRenderer.materialInstance)].push_back({modelMatrix, normalMatrix});
		});*/

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
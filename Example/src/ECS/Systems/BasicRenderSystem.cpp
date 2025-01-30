#include "pch.hpp"

#include "BasicRenderSystem.hpp"

std::vector<Render::InstanceGroup> PrepareInstanceGroups(ECS::ComponentManager& _componentManager, QueryVector query);

BasicRenderSystem::BasicRenderSystem(BasicRenderer* _renderer) : RenderSystem(_renderer)
{
	renderCameraBuffer = Helper::Memory::CreateBuffer(renderer->GetContext()->GetDevice(), renderer->GetContext()->GetPhysicalDevice(), sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, renderCameraBufferMemory);
}

void BasicRenderSystem::OnRegister(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager)
{
	renderCamera = _componentManager.FindFirstWith<Camera>();

	if (renderCamera != ECS::EntityManager::NULL_ENTITY)
	{
		renderer->UpdateRenderCameraBuffer(renderCameraBuffer);
	}

	directionalLightEntity = _componentManager.FindFirstWith<DirectionalLight>();

	if (directionalLightEntity != ECS::EntityManager::NULL_ENTITY)
	{
		auto& directionalLight = _componentManager.GetComponent<DirectionalLight>(directionalLightEntity);

		renderer->SetDirectionalLight(directionalLight);
	}
}

void BasicRenderSystem::Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt)
{
	assert(renderCamera != ECS::EntityManager::NULL_ENTITY, "Render camera cannot be null");

	auto& camera = _componentManager.GetComponent<Camera>(renderCamera);
	if (camera.Update(_componentManager.GetComponent<Transform>(renderCamera)))
	{
		Helper::Memory::MapMemory(renderer->GetContext()->GetDevice(), renderCameraBufferMemory, sizeof(glm::mat4), &camera.viewProjection);
	}

	if(query.empty())
	{
		for (auto [mesh, transform] : _componentManager.QueryArchetype<MeshRenderer, Transform>())
		{
			query.emplace_back(mesh, transform);
		}
	}

	auto instanceGroups = PrepareInstanceGroups(_componentManager, query);

	renderer->Render(instanceGroups);
}

void BasicRenderSystem::Cleanup()
{
	Helper::Memory::DestroyBuffer(renderer->GetContext()->GetDevice(), renderCameraBuffer, renderCameraBufferMemory);
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

	uint32_t instanceOffset = 0;

	for (auto& [tuple, tdata] : data)
	{
		auto [material, mesh, materialInstance] = tuple;

		instanceGroups.push_back({ material, materialInstance, mesh, tdata, instanceOffset });

		instanceOffset += tdata.size();
	}

	std::sort(instanceGroups.begin(), instanceGroups.end(), [](const Render::InstanceGroup& a, const Render::InstanceGroup& b)
		{
			return a.material < b.material && a.mesh < b.mesh && a.materialInstance < b.materialInstance;
		});

	return instanceGroups;
}

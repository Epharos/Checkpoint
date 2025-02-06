#include "pch.hpp"

#include "BasicRenderSystem.hpp"

BasicRenderSystem::BasicRenderSystem(BasicRenderer* _renderer) : RenderSystem(_renderer)
{
	//renderCameraBuffer = Helper::Memory::CreateBuffer(renderer->GetContext()->GetDevice(), renderer->GetContext()->GetPhysicalDevice(), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, renderCameraBufferMemory);
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
		auto& camera = _componentManager.GetComponent<Camera>(renderCamera);

		cascadeSplits = GetCascadeSplits(camera.near, camera.far, directionalLight.cascadeCount, 0.75f);
		frustumCorners = GetFrustumCorners(camera.cameraUBO.viewProjection);
		lightViewProjections = new glm::mat4[directionalLight.cascadeCount];
		lightViewProjections = GetCascadeProjections(camera.cameraUBO.projection, camera.cameraUBO.view, camera.cameraUBO.viewProjection, directionalLight.cascadeCount, camera.near, camera.far, directionalLight.direction);

		renderer->SetupDirectionalLight(vk::Extent2D(directionalLight.shadowMapSize, directionalLight.shadowMapSize), glm::vec4(directionalLight.color, directionalLight.intensity), directionalLight.direction, directionalLight.cascadeCount, cascadeSplits);
	}
}

void BasicRenderSystem::Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt)
{
	assert(renderCamera != ECS::EntityManager::NULL_ENTITY, "Render camera cannot be null");

	auto& camera = _componentManager.GetComponent<Camera>(renderCamera);
	if (camera.Update(_componentManager.GetComponent<Transform>(renderCamera)))
	{
		Helper::Memory::MapMemory(renderer->GetContext()->GetDevice(), renderCameraBufferMemory, sizeof(CameraUBO), &camera.cameraUBO);
	}

	auto& directionalLight = _componentManager.GetComponent<DirectionalLight>(directionalLightEntity);
	renderer->UpdateDirectionalLight(GetCascadeProjections(camera.cameraUBO.projection, camera.cameraUBO.view, camera.cameraUBO.viewProjection, directionalLight.cascadeCount, camera.near, camera.far, directionalLight.direction));

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

std::vector<Render::InstanceGroup> BasicRenderSystem::PrepareInstanceGroups(ECS::ComponentManager& _componentManager, QueryVector query)
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

float* BasicRenderSystem::GetCascadeSplits(const float& _near, const float& _far, const uint8_t& _cascadeCount, const float& _lambda)
{
	float* splits = new float[_cascadeCount];

	float range = _far - _near;
	float ratio = _far / _near;

	for (int i = 0; i < _cascadeCount; i++)
	{
		float p = (i + 1) / static_cast<float>(_cascadeCount);
		float log = _near * std::pow(ratio, p);
		float uniform = _near + range * p;
		float d = _lambda * (log - uniform) + uniform;
		splits[i] = (d - _near) / range;

		LOG_DEBUG(MF("Split #", i, ": ", splits[i]));
	}

	return splits;
}

glm::vec3* BasicRenderSystem::GetFrustumCorners(const glm::mat4& _viewProjection)
{
	glm::vec4 frustumCorners[8];
	glm::vec3* corners = new glm::vec3[8];

	glm::mat4 invViewProjection = glm::inverse(_viewProjection);

	frustumCorners[0] = invViewProjection * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
	frustumCorners[1] = invViewProjection * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	frustumCorners[2] = invViewProjection * glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
	frustumCorners[3] = invViewProjection * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
	frustumCorners[4] = invViewProjection * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);
	frustumCorners[5] = invViewProjection * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	frustumCorners[6] = invViewProjection * glm::vec4(1.0f, -1.0f, 1.0f, 1.0f);
	frustumCorners[7] = invViewProjection * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);

	for (int i = 0; i < 8; i++)
	{
		corners[i] = frustumCorners[i] / frustumCorners[i].w;

		//LOG_DEBUG(MF("Corner #", i, ": ", corners[i].x, ", ", corners[i].y, ", ", corners[i].z));
	}

	return corners;
}

glm::mat4* BasicRenderSystem::GetCascadeProjections(const glm::mat4 _cameraProj, const glm::mat4& _cameraView, const glm::mat4& _cameraViewproj, const uint32_t& _cascadeCount, const float& _near, const float& _far, const glm::vec3& _lightDir)
{
	delete[] frustumCorners;

	frustumCorners = GetFrustumCorners(_cameraViewproj);

	float lastSplit = 0.0f;

	for (int i = 0; i < _cascadeCount; i++)
	{
		float currentSplit = cascadeSplits[i];

		glm::vec3 splitCorners[8];

		for (int j = 0; j < 4; j++)
		{
			glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
			splitCorners[j + 4] = frustumCorners[j] + dist * currentSplit;
			splitCorners[j] = frustumCorners[j] + dist * lastSplit;
		}

		glm::vec3 center = glm::vec3(0.0f);
	
		for (const auto& corner : splitCorners)
		{
			center += corner;
		}

		center /= 8.0f;

		//LOG_DEBUG(MF("Cascade #", i, " center: ", center.x, ", ", center.y, ", ", center.z));

		float radius = 0.0f;

		for (const auto& corner : splitCorners)
		{
			float distance = glm::length(corner - center);
			radius = std::max(radius, distance);
		}

		radius = std::ceil(radius * 16.0f) / 16.0f; //wtf ?

		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		//LOG_DEBUG(MF("Min extents: ", minExtents.x, ", ", minExtents.y, ", ", minExtents.z));
		//LOG_DEBUG(MF("Max extents: ", maxExtents.x, ", ", maxExtents.y, ", ", maxExtents.z));

		glm::vec3 lightPos = center - _lightDir * -minExtents.z;

		//LOG_DEBUG(MF("Light position: ", lightPos.x, ", ", lightPos.y, ", ", lightPos.z));

		glm::mat4 lightView = glm::lookAtRH(lightPos, center, VEC3_UP);

		//LOG_DEBUG(MF("Light position: ", lightPos.x, ", ", lightPos.y, ", ", lightPos.z));

		/*glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());

		for (const auto& corner : splitCorners)
		{
			glm::vec4 lightSpaceCorner = lightView * glm::vec4(corner, 1.0f);

			min.x = std::min(min.x, lightSpaceCorner.x);
			min.y = std::min(min.y, lightSpaceCorner.y);
			min.z = std::min(min.z, lightSpaceCorner.z);

			max.x = std::max(max.x, lightSpaceCorner.x);
			max.y = std::max(max.y, lightSpaceCorner.y);
			max.z = std::max(max.z, lightSpaceCorner.z);
		}

		min.z = (min.z < 0.0f) ? min.z * 1.1f : min.z * 0.9f;
		max.z = (max.z < 0.0f) ? max.z * 0.9f : max.z * 1.1f;

		LOG_DEBUG(MF("Cascade #", i, " min: ", min.x, ", ", min.y, ", ", min.z));
		LOG_DEBUG(MF("Cascade #", i, " max: ", max.x, ", ", max.y, ", ", max.z));*/

		glm::mat4 lightProj = glm::orthoRH(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

		lastSplit = currentSplit;

		lightViewProjections[i] = lightProj * lightView;
	}

	return lightViewProjections;
}

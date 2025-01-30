#pragma once

#include "../../pch.hpp"
#include "../../BasicRenderer.hpp"

using QueryVector = std::vector<std::tuple<MeshRenderer&, Transform&>>;

template<typename R>
class RenderSystem : public ECS::System
{
protected:
	R* renderer;

	Entity renderCamera = ECS::EntityManager::NULL_ENTITY;
	vk::Buffer renderCameraBuffer;
	vk::DeviceMemory renderCameraBufferMemory;

public:
	RenderSystem(R* _renderer) : renderer(_renderer)
	{
		Render::Renderer* castedRenderer = dynamic_cast<Render::Renderer*>(_renderer);

		if (castedRenderer == nullptr)
		{
			LOG_ERROR("Renderer is not a valid Render::Renderer");
			throw std::runtime_error("Renderer is not a valid Render::Renderer");
			return;
		}

		renderCameraBuffer = Helper::Memory::CreateBuffer(castedRenderer->GetContext()->GetDevice(), castedRenderer->GetContext()->GetPhysicalDevice(), sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, renderCameraBufferMemory);
	}
	virtual void OnRegister(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager) = 0;

	virtual void Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt) = 0;
	virtual void Cleanup() = 0;
};
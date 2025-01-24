#pragma once

#include "pch.hpp"

struct SunLight
{
	glm::mat4 viewProjectionMatrix;
	glm::vec4 lightDirection;
	glm::vec4 lightColor;
};

class BasicRenderer : public Render::Renderer
{
protected:
	vk::Buffer instancedBuffer;
	vk::DeviceMemory instancedBufferMemory;

	SunLight sunLight;
	vk::Buffer sunLightBuffer;
	vk::DeviceMemory sunLightBufferMemory;

	Render::Camera* directionnalLight;
	Render::RenderTarget* shadowMapRT;

	const uint32_t MAX_RENDERABLE_ENTITIES = 1000;

	vk::RenderPass shadowMapRenderPass;

	void CreateMainRenderPass() override;
	void CreateRenderPasses() override;

	void RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	BasicRenderer(Context::VulkanContext* _context, const uint32_t& _maxRenderableEntities = 10000);
	~BasicRenderer();

	virtual void Cleanup() override;
};
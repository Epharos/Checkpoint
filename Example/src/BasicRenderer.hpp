#pragma once

#include "pch.hpp"

class BasicRenderer : public Render::Renderer
{
protected:
	vk::Buffer instancedBuffer;
	vk::DeviceMemory instancedBufferMemory;

	Render::Camera* directionnalLight;
	Render::RenderTarget* shadowMapRT;

	const uint32_t MAX_RENDERABLE_ENTITIES = 10000;

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
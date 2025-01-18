#pragma once

#include "pch.hpp"



class BasicRenderer : public Render::Renderer
{
protected:
	vk::Buffer instancedBuffer;
	vk::DeviceMemory instancedBufferMemory;

	Render::Camera* directionnalLight;

	const uint32_t MAX_RENDERABLE_ENTITIES = 10000;

	void CreateMainRenderPass() override;
	void AddRenderTargets() override;

	void RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	BasicRenderer() = default;
	BasicRenderer(const uint32_t& _maxRenderableEntities) : MAX_RENDERABLE_ENTITIES(_maxRenderableEntities) {}
	~BasicRenderer();

	virtual void Cleanup() override;
};
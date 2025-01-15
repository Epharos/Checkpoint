#pragma once

#include "pch.hpp"



class BasicRenderer : public Render::Renderer
{
protected:
	vk::Buffer instancedBuffer;
	vk::DeviceMemory instancedBufferMemory;

	void CreateMainRenderPass() override;

	void RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	BasicRenderer() = default;
	~BasicRenderer();

	virtual void Cleanup() override;
};
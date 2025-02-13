#pragma once

#include <Core.hpp>

class MinimalistRenderer : public Render::Renderer
{
protected:
	void CreateMainRenderPass() override;

	void RenderFrame(const std::vector<Render::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	MinimalistRenderer(Context::VulkanContext* _context);

	virtual void Cleanup() override;
};
#pragma once

#include <Core.hpp>

class MinimalistRenderer : public cp::Renderer
{
protected:
	cp::Mesh* quadMesh = nullptr;

	void CreateMainRenderPass() override;

	void RenderFrame(const std::vector<cp::InstanceGroup>& _instanceGroups) override;

	void SetupPipelines() override;

public:
	MinimalistRenderer(cp::VulkanContext* _context);

	virtual void Cleanup() override;
};
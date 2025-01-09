#pragma once

#include "pch.hpp"

class BasicRenderer : public Render::Renderer
{
protected:
	void CreateMainRenderPass() override;
	//void CreateRenderPasses() override;

	void RenderFrame(const std::vector<RenderCommand>& _commands) override;

	void SetupPipelines() override;

public:
	BasicRenderer() = default;
	~BasicRenderer();
};
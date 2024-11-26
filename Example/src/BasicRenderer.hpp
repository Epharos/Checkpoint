#pragma once

#include "pch.hpp"

class BasicRenderer : public Render::Renderer
{
protected:
	void CreateMainRenderPass() override;
	//void CreateRenderPasses() override;

	void RenderFrame() override;

public:
	BasicRenderer() = default;
	~BasicRenderer();
};
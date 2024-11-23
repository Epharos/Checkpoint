#pragma once

#include <Core.hpp>

class BasicRenderer : public Render::Renderer
{
protected:
	void CreateMainRenderPass() override;
	//void CreateRenderPasses() override;

public:
	BasicRenderer() = default;
	~BasicRenderer();

	void Render() override;
};
#include "pch.hpp"
#include "Renderer.hpp"

void Render::Renderer::CreateRenderPasses()
{

}

void Render::Renderer::Build(Context::VulkanContext* _context)
{
	context = _context;
	swapchain = new Swapchain(_context);
	CreateMainRenderPass();
	CreateRenderPasses();
	swapchain->Create(mainRenderPass);
}

Render::Renderer::~Renderer()
{
	
}

void Render::Renderer::Cleanup()
{
	delete swapchain;
	context->GetDevice().destroyRenderPass(mainRenderPass);
}

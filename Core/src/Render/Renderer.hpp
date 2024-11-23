#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"
#include "Swapchain.hpp"

namespace Render
{
	class Renderer
	{
	protected:
		Context::VulkanContext* context;
		Swapchain* swapchain;

		vk::RenderPass mainRenderPass;

		virtual void CreateMainRenderPass() = 0;
		virtual void CreateRenderPasses();

	public:
		Renderer() = default;
		virtual ~Renderer();

		virtual void Build(Context::VulkanContext* _context);

		virtual void Cleanup();

		virtual void Render() = 0;
	};
}
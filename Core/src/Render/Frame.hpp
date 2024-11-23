#pragma once

#include "../pch.hpp"

#include "RenderTarget.hpp"

namespace Render
{
	class Frame
	{
	private:
		std::vector<RenderTarget*> renderTargets;

		vk::CommandBuffer commandBuffer;

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;
		vk::Fence inFlightFence;

		Context::VulkanContext* context;

	public:
		Frame(Context::VulkanContext*& _context);
		~Frame();

		void AddRenderTarget(RenderTarget* _renderTarget);
	};
}
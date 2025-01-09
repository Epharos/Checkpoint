#pragma once

#include "../../pch.hpp"

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

		inline constexpr vk::Semaphore& GetImageAvailableSemaphore() { return imageAvailableSemaphore; }
		inline constexpr vk::Semaphore& GetRenderFinishedSemaphore() { return renderFinishedSemaphore; }
		inline constexpr vk::Fence& GetInFlightFence() { return inFlightFence; }
		inline constexpr vk::CommandBuffer& GetCommandBuffer() { return commandBuffer; }
		inline constexpr RenderTarget* GetRenderTarget(uint32_t _index) { return renderTargets[_index]; }
		inline constexpr std::vector<RenderTarget*>& GetRenderTargets() { return renderTargets; }
		inline constexpr RenderTarget* GetMainRenderTarget() { return renderTargets[0]; }
	};
}
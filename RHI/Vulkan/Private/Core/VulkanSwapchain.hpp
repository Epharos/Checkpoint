#pragma once

#include "../pch.hpp"

#include <ISwapchain.hpp>

namespace cp
{
	class VulkanSwapchain final : public ISwapchain
	{
	public:
		VulkanSwapchain(ILogger& _logger, const SwapchainInfo& _info);
		~VulkanSwapchain() override;

		void Present() override;
		void Resize(Extent2D<int> newExtent) override;
		uint32_t AcquireNextImage() override;

	private:
		void Initialize();
		void Cleanup();

	private:
		vk::SwapchainKHR swapchain;
	};
}
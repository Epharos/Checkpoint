#pragma once

#include "../pch.hpp"

#include <vector>

#include <ISwapchain.hpp>

#include "VulkanSurface.hpp"

namespace cp
{
	// Forward declarations
	class VulkanDevice;

	class VulkanSwapchain final : public ISwapchain
	{
	public:
		VulkanSwapchain(ILogger& _logger, const SwapchainInfo& _info, VulkanDevice& _device);
		~VulkanSwapchain() override;

		void Present() override;
		void Resize(Extent2D<int> newExtent) override;
		uint32_t AcquireNextImage() override;

	private:
		void Initialize();
		void Cleanup();

	private:
		void CreateSurface();
		void CreateSwapchain();

		void QuerrySurfaceProperties();
		void SelectSwapchainProperties();

	private:
		std::unique_ptr<VulkanSurface> surface;
		vk::SwapchainKHR swapchain { VK_NULL_HANDLE };

		VulkanDevice& device;

		// Cached surface properties
		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		std::vector<vk::PresentModeKHR> presentModes;

		// Chosen surface properties
		vk::SurfaceFormatKHR selectedSurfaceFormat;
		vk::PresentModeKHR selectedPresentMode;
	};
}
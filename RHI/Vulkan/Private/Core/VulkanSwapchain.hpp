#pragma once

#include "../pch.hpp"

#include <vector>
#include <memory>

#include <ISwapchain.hpp>

#include "VulkanSurface.hpp"

namespace cp
{
	// Forward declarations
	class VulkanDevice;
	class VulkanTexture;

	class VulkanSwapchain final : public ISwapchain
	{
	public:
		VulkanSwapchain(ILogger& _logger, const SwapchainInfo& _info, VulkanDevice& _device);
		~VulkanSwapchain() override;

		void Present() override;

		void Recreate();

		void Resize(Extent2D<int> newExtent) override;
		uint32_t AcquireNextImage() override;

	private:
		void Initialize();
		void Cleanup();

	private:
		/**
		 * @brief Creates a vk::Surface from the native window handle provided in the SwapchainInfo
		 */
		void CreateSurface();
		/**
		 * @brief Creates a vk::Swapchain from the provided SwapchainInfo structure
		 */
		void CreateSwapchain();

		/**
		 * @brief Creates the semaphores needed to acquire and present
		 */
		void CreateSynchronizationPrimitives();

		/**
		 * @brief Retrieves the vk::Image from the swapchain and stores them as our ITexture objects
		 */
		void RetrieveSwapchainImages();

		/**
		 * @brief Queries the properties (extent, accepted formats, ...) for the created vk::Surface
		 */
		void QuerySurfaceProperties();

		/**
		 * @brief Using the queried properties, selects the extent, surface format and present modes for this swapchain
		 */
		void SelectSwapchainProperties();

	private:
		std::unique_ptr<VulkanSurface> surface;
		vk::SwapchainKHR swapchain { VK_NULL_HANDLE };

		VulkanDevice& device;

		std::vector<std::unique_ptr<VulkanTexture>> swapchainImages;

		// Cached surface properties
		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		std::vector<vk::PresentModeKHR> presentModes;

		// Chosen surface properties
		vk::SurfaceFormatKHR selectedSurfaceFormat;
		vk::PresentModeKHR selectedPresentMode;

		vk::Semaphore imageAvailableSemaphore { VK_NULL_HANDLE };
		vk::Semaphore renderFinishedSemaphore { VK_NULL_HANDLE };

		uint32_t imageIndex;
	};
}
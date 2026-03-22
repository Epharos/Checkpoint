#pragma once

#include "../pch.hpp"

#include <vector>
#include <memory>

#include <RHI/Rendering.hpp>
#include <Common/Core/Assert.hpp>

#include "Surface.hpp"

namespace cp
{
	// Forward declarations
	class Device;
	class Texture;

	class Swapchain final : public ISwapchain
	{
	public:
		Swapchain(ILogger& _logger, const SwapchainInfo& _info, Device& _device);
		~Swapchain() override;

		/**
		 * @brief Updates the swapchain image state to present it within the surface (i.e. the window) and swap the back buffer.
		 */
		void Present(size_t _synchronizationSignalValue) override;

		/**
		 * @brief Recreates the whole swapchain and related objects (semaphores, ...).
		 */
		void Recreate();

		/**
		 * @brief Resize the swapchain and recreates it.
		 * @param newExtent The new extent to resize the swapchain to.
		 */
		void Resize(Extent2D<int> newExtent) override;

		/**
		 * @brief Acquires next swapchain image index and handles synchronization.
		 * @return The index of the next back buffer image.
		 */
		uint32_t AcquireNextImage() override;

		[[nodiscard]] vk::SwapchainKHR& GetHandle() { return swapchain; }
		[[nodiscard]] const vk::SwapchainKHR& GetHandle() const { return swapchain; }

		[[nodiscard]] Surface& GetSurface() { CP_EXPECT_MSG(surface, "Surface cannot be null"); return *surface; }
		[[nodiscard]] const Surface& GetSurface() const { CP_EXPECT_MSG(surface, "Surface cannot be null"); return *surface; }

	private:
		void Initialize();
		void Cleanup();

	private:
		/**
		 * @brief Creates a vk::Surface from the native window handle provided in the SwapchainInfo.
		 */
		void CreateSurface();
		/**
		 * @brief Creates a vk::Swapchain from the provided SwapchainInfo structure.
		 */
		void CreateSwapchain();

		/**
		 * @brief Creates the semaphores needed to acquire and present.
		 */
		void CreateSynchronizationPrimitives();

		/**
		 * @brief Retrieves the vk::Image from the swapchain and stores them as our ITexture objects.
		 */
		void RetrieveSwapchainImages();

		/**
		 * @brief Queries the properties (extent, accepted formats, ...) for the created vk::Surface.
		 */
		void QuerySurfaceProperties();

		/**
		 * @brief Using the queried properties, selects the extent, surface format and present modes for this swapchain.
		 */
		void SelectSwapchainProperties();

	public:
		ITexture& GetSwapchainImage(size_t _index) override;

	private:
		std::unique_ptr<Surface> surface;
		vk::SwapchainKHR swapchain { VK_NULL_HANDLE };

		Device& device;

		std::vector<std::unique_ptr<Texture>> swapchainImages;

		// Cached surface properties
		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		std::vector<vk::PresentModeKHR> presentModes;

		// Chosen surface properties
		vk::SurfaceFormatKHR selectedSurfaceFormat;
		vk::PresentModeKHR selectedPresentMode;

		std::vector<vk::Semaphore> imageAvailableBinarySemaphore;
		std::vector<vk::Semaphore> renderFinishedBinarySemaphore;

		uint32_t imageIndex{};
		uint32_t acquireSemaphoreIndex{};
	};
}
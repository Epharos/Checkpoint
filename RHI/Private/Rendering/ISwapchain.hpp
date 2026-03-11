#pragma once

#include <memory>
#include <Common/Core/Assert.hpp>
#include <Common/Data/Extent.hpp>
#include <RHI/Synchro.hpp>

namespace cp
{
	class ITexture;
	enum class Format : uint32_t;
	class ITimelineSemaphore;
	class ILogger;

	struct SwapchainInfo
	{
		Extent2D<int> extent;

		uint32_t imageCount;

		Format format;

		void* nativeWindowHandle = nullptr;
	};

	class ISwapchain
	{
	public:
		ISwapchain(ILogger& _logger, const SwapchainInfo& _info);
		virtual ~ISwapchain() = default;

		virtual void Present(size_t _synchronizationSignalValue) = 0;
		virtual void Resize(Extent2D<int> newExtent) = 0;

		virtual uint32_t AcquireNextImage() = 0;

		virtual ITexture& GetSwapchainImage(size_t _index) = 0;

		[[nodiscard]] const SwapchainInfo& GetInfo() const { return info; }

		[[nodiscard]] Format GetImageFormat() const { return info.format; }
		[[nodiscard]] Extent2D<int> GetImageExtent() const { return info.extent; }
		[[nodiscard]] uint32_t GetImageCount() const { return info.imageCount; }

		/**
		* @brief Returns the timeline semaphore signaled when the next swapchain image is available for rendering.
		*        Use GetLastImageAvailableSignalValue() to get the value to wait for.
		*
		* @return The image available timeline semaphore.
		*/
		[[nodiscard]] ITimelineSemaphore& GetImageAvailableSemaphore() const
		{
			CP_ENSURE_MSG(imageAvailableTimelineSemaphore, "Image Available Semaphore is null");
			return *imageAvailableTimelineSemaphore;
		}

		/**
		* @brief Returns the timeline semaphore that must be signaled when rendering to the current swapchain image is complete.
		*        Present() will wait on this semaphore before presenting.
		*
		* @return The render finished timeline semaphore.
		*/
		[[nodiscard]] ITimelineSemaphore& GetRenderFinishedSemaphore() const
		{
			CP_ENSURE_MSG(renderFinishedTimelineSemaphore, "Render Finished Semaphore is null");
			return *renderFinishedTimelineSemaphore;
		}

		/**
		 * @brief Returns the timeline semaphore value that was submitted as a signal in the most recent AcquireNextImage call.
		 *        Use this as the wait value when submitting work that depends on image availability.
		 */
		[[nodiscard]] uint64_t GetLastImageAvailableSignalValue() const { return lastImageAvailableSignalValue; }

	protected:
		ILogger& logger;

		SwapchainInfo info;

		std::unique_ptr<ITimelineSemaphore> imageAvailableTimelineSemaphore;
		std::unique_ptr<ITimelineSemaphore> renderFinishedTimelineSemaphore;

		uint64_t lastImageAvailableSignalValue = 0;
	};
}
#pragma once

#include <Extent.hpp>

#include "../Data/Formats.hpp"

namespace cp
{
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

		virtual void Present() = 0;
		virtual void Resize(Extent2D<int> newExtent) = 0;

		virtual uint32_t AcquireNextImage() = 0;

		[[nodiscard]] const SwapchainInfo& GetInfo() const { return info; }

		Format GetImageFormat() const { return info.format; }
		Extent2D<int> GetImageExtent() const { return info.extent; }
		uint32_t GetImageCount() const { return info.imageCount; }

	protected:
		SwapchainInfo info;

		ILogger& logger;
	};
}
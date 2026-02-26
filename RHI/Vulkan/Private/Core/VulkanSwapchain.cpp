#include "../pch.hpp"

#include "VulkanSwapchain.hpp"

#include <Assert.hpp>
#include <Log.hpp>

#include "VulkanInstance.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanDevice.hpp"

#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	namespace
	{
		Extent2D<int> SelectSwapExtent(
			const vk::SurfaceCapabilitiesKHR& surfaceCapabilities, 
			const Extent2D<int>& desiredExtent
		)
		{
#pragma push_macro("max")
#undef max
			if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				auto vkExtent = surfaceCapabilities.currentExtent;
				return { static_cast<int>(vkExtent.width), static_cast<int>(vkExtent.height) };
			}
#pragma pop_macro("max")

			Extent2D<int> actualExtent = {
				std::clamp(desiredExtent.x(), static_cast<int>(surfaceCapabilities.minImageExtent.width), static_cast<int>(surfaceCapabilities.maxImageExtent.width)),
				std::clamp(desiredExtent.y(), static_cast<int>(surfaceCapabilities.minImageExtent.height), static_cast<int>(surfaceCapabilities.maxImageExtent.height))
			};

			return actualExtent;
		}

		vk::PresentModeKHR SelectPresentMode(
			const std::vector<vk::PresentModeKHR>& availablePresentModes, 
			vk::PresentModeKHR desiredPresentMode
		)
		{
			for (const auto& presentMode : availablePresentModes)
			{
				if (presentMode == desiredPresentMode)
				{
					return presentMode;
				}
			}

			return vk::PresentModeKHR::eFifo; // Guaranteed to be available
		}

		vk::SurfaceFormatKHR SelectSurfaceFormat(
			const std::vector<vk::SurfaceFormatKHR>& availableFormats, 
			vk::SurfaceFormatKHR desiredFormat
		)
		{
			for (const auto& format : availableFormats)
			{
				if (format.format == desiredFormat.format && format.colorSpace == desiredFormat.colorSpace)
				{
					return format;
				}
			}

			return availableFormats[0]; // If the desired format isn't available, just pick the first one
		}
	}

	cp::VulkanSwapchain::VulkanSwapchain(ILogger& _logger, const SwapchainInfo& _info, VulkanDevice& _device)
		: ISwapchain(_logger, _info), device(_device)
	{
		Initialize();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		Cleanup();
	}

	void VulkanSwapchain::Present()
	{
		throw std::logic_error("Not implemented yet.");
	}

	void VulkanSwapchain::Resize(Extent2D<int> newExtent)
	{
		throw std::logic_error("Not implemented yet.");
	}

	uint32_t VulkanSwapchain::AcquireNextImage()
	{
		throw std::logic_error("Not implemented yet.");
		return 0;
	}

	void VulkanSwapchain::Initialize()
	{
		CreateSurface();
		QuerrySurfaceProperties();

		SelectSwapchainProperties();

		CreateSwapchain();
	}

	void VulkanSwapchain::Cleanup()
	{
		if (swapchain != VK_NULL_HANDLE)
		{
			device.GetHandle().destroySwapchainKHR(swapchain);
		}
	}

	void VulkanSwapchain::CreateSurface()
	{
		CP_EXPECT_MSG(info.nativeWindowHandle, "Native window handle must be provided to create a swapchain");
		CP_EXPECT_MSG(!surface, "Surface already created for this swapchain");

		SurfaceInfo surfaceInfo;
		surfaceInfo.nativeWindowHandle = info.nativeWindowHandle;

		surface = std::make_unique<VulkanSurface>(logger, surfaceInfo, device.GetPhysicalDevice().GetInstance());

		CP_ENSURE_MSG(surface, "Failed to create Vulkan surface for swapchain");
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		const uint32_t imageCount = std::clamp(info.imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

		vk::SwapchainCreateInfoKHR createInfo = {};
		createInfo.setSurface(surface->GetHandle());
		createInfo.setMinImageCount(imageCount);
		createInfo.setImageFormat(selectedSurfaceFormat.format);
		createInfo.setImageColorSpace(selectedSurfaceFormat.colorSpace);
		createInfo.setImageExtent(vk::Extent2D{ static_cast<uint32_t>(info.extent.x()), static_cast<uint32_t>(info.extent.y()) });
		createInfo.setImageArrayLayers(1);
		createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

		const VulkanQueueFamilies& queues = device.GetQueueFamilies();

#pragma push_macro("max")
#undef max
		CP_ASSERT_MSG(queues.graphics != std::numeric_limits<uint32_t>::max(), "Graphics queue family index must be valid to create a swapchain");
		CP_ASSERT_MSG(queues.present != std::numeric_limits<uint32_t>::max(), "Present queue family index must be valid to create a swapchain");
#pragma pop_macro("max")

		uint32_t queueFamilyIndices[] = { queues.graphics, queues.present };

		if(queues.graphics != queues.present)
		{
			createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			createInfo.setQueueFamilyIndexCount(2);
			createInfo.setPQueueFamilyIndices(queueFamilyIndices);
		}
		else
		{
			createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
		}

		createInfo.setPreTransform(surfaceCapabilities.currentTransform);
		createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		createInfo.setPresentMode(selectedPresentMode);
		createInfo.setClipped(true);
		createInfo.setOldSwapchain(VK_NULL_HANDLE);

		swapchain = device.GetHandle().createSwapchainKHR(createInfo);

		CP_ENSURE_MSG(swapchain, "Failed to create Vulkan swapchain");
	}

	void VulkanSwapchain::QuerrySurfaceProperties()
	{
		const vk::PhysicalDevice physicalDevice = device.GetPhysicalDevice().GetHandle();
		const vk::SurfaceKHR surfaceHandle = surface->GetHandle();

		CP_EXPECT_MSG(physicalDevice, "Physical device must be created before querrying surface properties");
		CP_EXPECT_MSG(surfaceHandle, "Surface must be created before querrying its properties");

		surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surfaceHandle);
		surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surfaceHandle);
		presentModes = physicalDevice.getSurfacePresentModesKHR(surfaceHandle);

		CP_ENSURE_MSG(!surfaceFormats.empty(), "No surface formats available for the created surface");
		CP_ENSURE_MSG(!presentModes.empty(), "No present modes available for the created surface");
	}

	void VulkanSwapchain::SelectSwapchainProperties()
	{
		info.extent = SelectSwapExtent(surfaceCapabilities, info.extent);
		selectedPresentMode = SelectPresentMode(presentModes, vk::PresentModeKHR::eMailbox);
		selectedSurfaceFormat = SelectSurfaceFormat(surfaceFormats, { EnumCast<vk::Format, Format>(info.format), vk::ColorSpaceKHR::eSrgbNonlinear });

		// TODO : Let the user choose Present Mode and Surface Format instead of hardcoding them
	}

}
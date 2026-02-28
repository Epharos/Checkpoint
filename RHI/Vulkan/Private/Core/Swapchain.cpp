#include "../pch.hpp"

#include "Swapchain.hpp"

#include <Assert.hpp>
#include <Log.hpp>

#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "../Data/Texture.hpp"

#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	namespace
	{
		Extent2D<int> SelectSwapExtent(
			const vk::SurfaceCapabilitiesKHR& _surfaceCapabilities,
			const Extent2D<int>& _desiredExtent
		)
		{
			if (_surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				auto vkExtent = _surfaceCapabilities.currentExtent;
				return { static_cast<int>(vkExtent.width), static_cast<int>(vkExtent.height) };
			}

			Extent2D<int> actualExtent = {
				std::clamp(
					_desiredExtent.x(),
					static_cast<int>(_surfaceCapabilities.minImageExtent.width),
					static_cast<int>(_surfaceCapabilities.maxImageExtent.width)
				),
				std::clamp(
					_desiredExtent.y(),
					static_cast<int>(_surfaceCapabilities.minImageExtent.height),
					static_cast<int>(_surfaceCapabilities.maxImageExtent.height)
				)
			};

			return actualExtent;
		}

		vk::PresentModeKHR SelectPresentMode(
			const std::vector<vk::PresentModeKHR>& _availablePresentModes,
			vk::PresentModeKHR _desiredPresentMode
		)
		{
			for (const auto& presentMode : _availablePresentModes)
			{
				if (presentMode == _desiredPresentMode)
				{
					return presentMode;
				}
			}

			return vk::PresentModeKHR::eFifo; // Guaranteed to be available
		}

		vk::SurfaceFormatKHR SelectSurfaceFormat(
			const std::vector<vk::SurfaceFormatKHR>& _availableFormats,
			vk::SurfaceFormatKHR _desiredFormat
		)
		{
			for (const auto& format : _availableFormats)
			{
				if (format.format == _desiredFormat.format && format.colorSpace == _desiredFormat.colorSpace)
				{
					return format;
				}
			}

			return _availableFormats[0]; // If the desired format isn't available, just pick the first one
		}
	}

	cp::Swapchain::Swapchain(ILogger& _logger, const SwapchainInfo& _info, Device& _device)
		: ISwapchain(_logger, _info), device(_device)
	{
		Initialize();
	}

	Swapchain::~Swapchain()
	{
		Cleanup();
	}

	void Swapchain::Present()
	{
		Queue& graphicsQueue = static_cast<Queue&>(device.GetQueue(QueueType::Graphics, 0));

		vk::Result result = vk::Result::eSuccess;

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.setSwapchainCount(1);
		presentInfo.setPSwapchains(&swapchain);
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(&renderFinishedSemaphore);
		presentInfo.setPImageIndices(&imageIndex);

		try
		{
			result = graphicsQueue.GetHandle().presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError e)
		{
			Recreate();
		}

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			Recreate();
		}
		else if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to present swapchain image");
		}

		imageIndex++;

		if (imageIndex >= info.imageCount) imageIndex = 0;
	}

	void Swapchain::Recreate()
	{
		device.WaitForIdle();

		Cleanup();

		QuerySurfaceProperties();

		SelectSwapchainProperties();

		CreateSwapchain();
		RetrieveSwapchainImages();

		CreateSynchronizationPrimitives();

		imageIndex = 0;
	}

	void Swapchain::Resize(Extent2D<int> newExtent)
	{
		info.extent = newExtent;
		Recreate();
	}

	uint32_t Swapchain::AcquireNextImage()
	{
		uint32_t result;

		try
		{
			result = device.GetHandle().acquireNextImageKHR(
				swapchain,
				std::numeric_limits<uint32_t>::max(),
				imageAvailableSemaphore)
			.value;
		}
		catch (vk::OutOfDateKHRError)
		{
			Recreate();
			return static_cast<uint32_t>(-1);
		}

		imageIndex = result;
		return result;
	}

	void Swapchain::Initialize()
	{
		CreateSurface();
		Recreate(); // Technically, we create, not recreate here but it's okay
	}

	void Swapchain::Cleanup()
	{
		if (swapchain != VK_NULL_HANDLE) device.GetHandle().destroySwapchainKHR(swapchain);

		swapchainImages.clear();

		if (imageAvailableSemaphore != VK_NULL_HANDLE) device.GetHandle().destroySemaphore(imageAvailableSemaphore);
		if (renderFinishedSemaphore != VK_NULL_HANDLE) device.GetHandle().destroySemaphore(renderFinishedSemaphore);
	}

	void Swapchain::CreateSurface()
	{
		CP_EXPECT_MSG(info.nativeWindowHandle, "Native window handle must be provided to create a swapchain");
		CP_EXPECT_MSG(!surface, "Surface already created for this swapchain");

		SurfaceInfo surfaceInfo;
		surfaceInfo.nativeWindowHandle = info.nativeWindowHandle;

		surface = std::make_unique<Surface>(logger, surfaceInfo, device.GetPhysicalDevice().GetInstance());

		CP_ENSURE_MSG(surface, "Failed to create Vulkan surface for swapchain");
	}

	void Swapchain::CreateSwapchain()
	{
		info.imageCount = std::clamp(info.imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

		vk::SwapchainCreateInfoKHR createInfo = {};
		createInfo.setSurface(surface->GetHandle());
		createInfo.setMinImageCount(info.imageCount);
		createInfo.setImageFormat(selectedSurfaceFormat.format);
		createInfo.setImageColorSpace(selectedSurfaceFormat.colorSpace);
		createInfo.setImageExtent(vk::Extent2D{
			static_cast<uint32_t>(info.extent.x()),
			static_cast<uint32_t>(info.extent.y())
		});
		createInfo.setImageArrayLayers(1);
		createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

		const VulkanQueueFamilies& queues = device.GetQueueFamilies();

		CP_ASSERT_MSG(queues.graphics != std::numeric_limits<uint32_t>::max(),
			"Graphics queue family index must be valid to create a swapchain"
		);
		CP_ASSERT_MSG(queues.present != std::numeric_limits<uint32_t>::max(),
			"Present queue family index must be valid to create a swapchain"
		);

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

	void Swapchain::CreateSynchronizationPrimitives()
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo = {};

		imageAvailableSemaphore = device.GetHandle().createSemaphore(semaphoreCreateInfo);
		renderFinishedSemaphore = device.GetHandle().createSemaphore(semaphoreCreateInfo);
	}

	void Swapchain::RetrieveSwapchainImages()
	{
		std::vector<vk::Image> swapchainNativeImages = device.GetHandle().getSwapchainImagesKHR(swapchain);

		for (vk::Image& image : swapchainNativeImages)
		{
			cp::TextureInfo texInfo
			{
				.type = cp::TextureType::Texture2D,
				.extent = Extent3D<uint32_t> {
					static_cast<uint32_t>(info.extent.x()),
					static_cast<uint32_t>(info.extent.y()),
					1
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				.format = EnumCast<Format, vk::Format>(selectedSurfaceFormat.format),
				.usage = cp::TextureUsage::ColorAttachment,
				.aspect = cp::TextureAspect::Color
			};

			swapchainImages.emplace_back(new Texture(logger, image, texInfo, device));
		}

		CP_ENSURE_MSG(swapchainImages.size() == swapchainNativeImages.size(),
			"The swapchain object stores a different count of image than what is returned by the vkSwapchain"
		);

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>(
			"Swapchain was created with {} images", swapchainImages.size()
			)));
	}

	void Swapchain::QuerySurfaceProperties()
	{
		const vk::PhysicalDevice physicalDevice = device.GetPhysicalDevice().GetHandle();
		const vk::SurfaceKHR surfaceHandle = surface->GetHandle();

		CP_EXPECT_MSG(physicalDevice, "Physical device must be created before querying surface properties");
		CP_EXPECT_MSG(surfaceHandle, "Surface must be created before querying its properties");

		surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surfaceHandle);
		surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surfaceHandle);
		presentModes = physicalDevice.getSurfacePresentModesKHR(surfaceHandle);

		CP_ENSURE_MSG(!surfaceFormats.empty(), "No surface formats available for the created surface");
		CP_ENSURE_MSG(!presentModes.empty(), "No present modes available for the created surface");
	}

	void Swapchain::SelectSwapchainProperties()
	{
		info.extent = SelectSwapExtent(surfaceCapabilities, info.extent);
		selectedPresentMode = SelectPresentMode(presentModes, vk::PresentModeKHR::eMailbox);
		selectedSurfaceFormat = SelectSurfaceFormat(surfaceFormats, { EnumCast<vk::Format, Format>(info.format), vk::ColorSpaceKHR::eSrgbNonlinear });

		// TODO : Let the user choose Present Mode and Surface Format instead of hardcoding them
	}

}
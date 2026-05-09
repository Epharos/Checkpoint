#include "../pch.hpp"

#include "../Rendering/Swapchain.hpp"

#include <Common/Core/Log.hpp>

#include "../Core/Instance.hpp"
#include "../Core/PhysicalDevice.hpp"
#include "../Core/Device.hpp"
#include "../Data/Texture.hpp"
#include "../Synchro/TimelineSemaphore.hpp"

#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	namespace
	{
		Extent2D<int> SelectExtent(
			const vk::SurfaceCapabilitiesKHR& _surfaceCapabilities,
			const Extent2D<int>& _desiredExtent
		)
		{
			if (_surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				const auto vkExtent = _surfaceCapabilities.currentExtent;
				return Extent2D{ static_cast<int>(vkExtent.width), static_cast<int>(vkExtent.height) };
			}

			const auto actualExtent = Extent2D{
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

	Swapchain::Swapchain(ILogger& _logger, const SwapchainInfo& _info, Device& _device)
		: ISwapchain(_logger, _info), device(_device)
	{
		Initialize();
	}

	Swapchain::~Swapchain()
	{
		Cleanup();
	}

	void Swapchain::Present(size_t _synchronizationSignalValue)
	{
		auto& graphicsQueue = dynamic_cast<Queue&>(device.GetQueue(QueueType::Graphics, 0));

		CP_EXPECT_MSG(renderFinishedTimelineSemaphore, "No Render Finished Timeline semaphore setup");

		graphicsQueue.SubmitTimelineToBinary(
			reinterpret_cast<TimelineSemaphore&>(*renderFinishedTimelineSemaphore).GetHandle(),
			_synchronizationSignalValue,
			renderFinishedBinarySemaphore[_synchronizationSignalValue % info.imageCount]
		);

		vk::Result result = vk::Result::eSuccess;

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.setSwapchainCount(1);
		presentInfo.setPSwapchains(&swapchain);
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(&renderFinishedBinarySemaphore[_synchronizationSignalValue % info.imageCount]);
		presentInfo.setPImageIndices(&imageIndex);

		try
		{
			result = graphicsQueue.Present(presentInfo);
		}
		catch (vk::OutOfDateKHRError&)
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
		device.WaitIdle();

		Cleanup();

		imageAvailableTimelineSemaphore = std::make_unique<TimelineSemaphore>(device);
		renderFinishedTimelineSemaphore = std::make_unique<TimelineSemaphore>(device);

		QuerySurfaceProperties();

		SelectSwapchainProperties();

		CreateSwapchain();
		RetrieveSwapchainImages();

		CreateSynchronizationPrimitives();

		imageIndex = 0;
		acquireSemaphoreIndex = 0;
		lastImageAvailableSignalValue = 0;
	}

	void Swapchain::Resize(Extent2D<int> newExtent)
	{
		info.extent = newExtent;
		Recreate();
	}

	uint32_t Swapchain::AcquireNextImage()
	{
		CP_EXPECT_MSG(imageAvailableTimelineSemaphore, "No Image Available Timeline semaphore setup");

		uint32_t result;

		try
		{
			result = device.GetHandle().acquireNextImageKHR(
				swapchain,
				std::numeric_limits<uint32_t>::max(),
				imageAvailableBinarySemaphore[acquireSemaphoreIndex])
			.value;

			reinterpret_cast<Queue&>(device.GetQueue(QueueType::Graphics, 0)).SubmitBinaryToTimeline(
				imageAvailableBinarySemaphore[acquireSemaphoreIndex],
				reinterpret_cast<TimelineSemaphore&>(*imageAvailableTimelineSemaphore).GetHandle(),
				++lastImageAvailableSignalValue
			);

			acquireSemaphoreIndex = (acquireSemaphoreIndex + 1) % info.imageCount;
		}
		catch (vk::OutOfDateKHRError&)
		{
			Recreate();
			return static_cast<uint32_t>(-1);
		}

		imageIndex = result;

		CP_ENSURE_MSG(imageIndex < info.imageCount, "Image index is more than the image count the swapchain should have");

		return result;
	}

	void Swapchain::Initialize()
	{
		CreateSurface();
		Recreate(); // Technically, we create, not recreate here but it's okay
	}

	void Swapchain::Cleanup()
	{
		device.WaitIdle();

		if (swapchain != VK_NULL_HANDLE) device.GetHandle().destroySwapchainKHR(swapchain);

		swapchainImages.clear();

		if (!imageAvailableBinarySemaphore.empty())
		{
			for (size_t i = 0; i < info.imageCount; ++i)
				device.GetHandle().destroySemaphore(imageAvailableBinarySemaphore[i]);

			imageAvailableBinarySemaphore.clear();
		}

		if (!renderFinishedBinarySemaphore.empty())
		{
			for (size_t i = 0; i < info.imageCount; ++i)
				device.GetHandle().destroySemaphore(renderFinishedBinarySemaphore[i]);

			renderFinishedBinarySemaphore.clear();
		}
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
		createInfo.setImageUsage(
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferSrc |
			vk::ImageUsageFlagBits::eTransferDst
		);

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
		CP_EXPECT_MSG(imageAvailableBinarySemaphore.empty(), "Binary semaphore for image availability is not empty");
		CP_EXPECT_MSG(renderFinishedBinarySemaphore.empty(), "Binary semaphore for render state is not empty");

		constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo = {};

		imageAvailableBinarySemaphore.resize(info.imageCount);
		renderFinishedBinarySemaphore.resize(info.imageCount);

		for (size_t i = 0; i < info.imageCount; ++i)
		{
			imageAvailableBinarySemaphore[i] = device.GetHandle().createSemaphore(semaphoreCreateInfo);
			renderFinishedBinarySemaphore[i] = device.GetHandle().createSemaphore(semaphoreCreateInfo);
		}
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

		// logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>(
		// 	"Swapchain was created with {} images", swapchainImages.size()
		// 	)));
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
		info.extent = SelectExtent(surfaceCapabilities, info.extent);
		selectedPresentMode = SelectPresentMode(presentModes, vk::PresentModeKHR::eMailbox);
		selectedSurfaceFormat = SelectSurfaceFormat(surfaceFormats, { EnumCast<vk::Format, Format>(info.format), vk::ColorSpaceKHR::eSrgbNonlinear });

		// TODO : Let the user choose Present Mode and Surface Format instead of hardcoding them
	}

	ITexture& Swapchain::GetSwapchainImage(size_t _index)
	{
		CP_EXPECT_MSG(_index < swapchainImages.size(), "Index is higher than the number of images in the swapchain");

		return *swapchainImages[_index];
	}
}

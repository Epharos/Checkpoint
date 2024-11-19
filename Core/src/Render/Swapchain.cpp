#include "Swapchain.hpp"

#include "pch.hpp"

void Render::Swapchain::CreateData()
{
	QuerySupport();

	auto surfaceFormat = SelectSurfaceFormat(surfaceFormats);
	auto presentMode = SelectPresentMode(presentModes);

	auto windowExtent = context->GetPlatform()->GetExtent();
	extent = SelectExtent(surfaceCapabilities, windowExtent.width, windowExtent.height);

	uint32_t imageCount = std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = context->GetSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	auto indices = context->GetQueueFamilyIndices();
	uint32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	
	if (indices.graphicsFamily.value() != indices.presentFamily.value())
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = nullptr;

	swapchain = context->GetDevice().createSwapchainKHR(createInfo);

	format = surfaceFormat.format;

	std::vector<vk::Image> images = context->GetDevice().getSwapchainImagesKHR(swapchain);
	frames.reserve(images.size());

	for (auto image : images)
	{
		//frames.push_back(new Frame(context, image, format, extent));
		//TODO : Fill frames
	}
}

void Render::Swapchain::QuerySupport()
{
	auto physicalDevice = context->GetPhysicalDevice();

	surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(context->GetSurface());
	surfaceFormats = physicalDevice.getSurfaceFormatsKHR(context->GetSurface());
	presentModes = physicalDevice.getSurfacePresentModesKHR(context->GetSurface());
}

vk::SurfaceFormatKHR Render::Swapchain::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& _formats, vk::Format _format, vk::ColorSpaceKHR _colorSpace)
{
	for (const auto& format : _formats)
	{
		if (format.format == _format && format.colorSpace == _colorSpace)
		{
			return format;
		}
	}

	return _formats[0];
}

vk::PresentModeKHR Render::Swapchain::SelectPresentMode(const std::vector<vk::PresentModeKHR>& _presentModes, vk::PresentModeKHR _presentMode)
{
	for (const auto& presentMode : _presentModes)
	{
		if (presentMode == _presentMode)
		{
			return presentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Render::Swapchain::SelectExtent(const vk::SurfaceCapabilitiesKHR& _capabilities, const uint32& _width, const uint32& _height)
{
	if (_capabilities.currentExtent.width != std::numeric_limits<uint32>().max())
	{
		return _capabilities.currentExtent;
	}

	vk::Extent2D actualExtent = { _width, _height };

	actualExtent.width = std::max(_capabilities.minImageExtent.width, std::min(_capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(_capabilities.minImageExtent.height, std::min(_capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

Render::Swapchain::Swapchain(Context::VulkanContext* _context)
{
	context = _context;
	Create();
}

Render::Swapchain::~Swapchain()
{
	Cleanup();
}

void Render::Swapchain::Create()
{
	CreateData();
	maxFramesInFlight = static_cast<uint32>(frames.size());
}

void Render::Swapchain::Recreate()
{
	auto windowExtent = context->GetPlatform()->GetExtent();

	while (windowExtent.width == 0 || windowExtent.height == 0)
	{
		extent = windowExtent;
		glfwWaitEvents(); //GLFW Magic
	}

	context->GetDevice().waitIdle();

	Cleanup();
	Create();
}

void Render::Swapchain::Cleanup()
{
	context->GetDevice().destroySwapchainKHR(swapchain);

	for (auto frame : frames)
	{
		delete frame;
	}
}

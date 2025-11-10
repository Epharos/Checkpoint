#include "pch.hpp"

#include "Swapchain.hpp"

#include "Frame.hpp"
#include "Helpers/Helpers.hpp"

#ifdef IN_EDITOR
#include <QtCore/qapplicationstatic.h>
#endif

void cp::Swapchain::CreateData()
{
	uint32_t imageCount = std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
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

	frames.clear();

	std::vector<vk::Image> images = context->GetDevice().getSwapchainImagesKHR(swapchain);
	frames.reserve(images.size());

	auto depthRTA = std::make_shared<RenderTargetAttachment>(context, extent, 
			Helper::Format::FindDepthFormat(context->GetPhysicalDevice()), vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth);

	for (auto image : images)
	{
		Frame* frame = new Frame(context);
		RenderTarget* rt = new RenderTarget(*context, extent, mainRenderPass);
		frame->AddRenderTarget(rt);
		auto colorRTA = std::make_shared<RenderTargetAttachment>(context, image, surfaceFormat.format, vk::ImageAspectFlagBits::eColor);
		colorRTA->isSwapchain = true;
		rt->AddAttachment(depthRTA);
		rt->AddAttachment(colorRTA);
		rt->Build();
		
		frames.push_back(frame);
	}
}

void cp::Swapchain::QuerySupport()
{
	auto physicalDevice = context->GetPhysicalDevice();

	surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
	presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
}

vk::SurfaceFormatKHR cp::Swapchain::SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& _formats, vk::Format _format, vk::ColorSpaceKHR _colorSpace)
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

vk::PresentModeKHR cp::Swapchain::SelectPresentMode(const std::vector<vk::PresentModeKHR>& _presentModes, vk::PresentModeKHR _presentMode)
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

vk::Extent2D cp::Swapchain::SelectExtent(const vk::SurfaceCapabilitiesKHR& _capabilities, const uint32& _width, const uint32& _height)
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

cp::Swapchain::Swapchain(cp::VulkanContext* _context, vk::SurfaceKHR& _surface, cp::Platform* _platform) : surface(_surface)
{
	context = _context;
	platform = _platform;
	Setup();
}

cp::Swapchain::~Swapchain()
{
	Cleanup();
}

void cp::Swapchain::Setup()
{
	QuerySupport();

	auto newSurfaceFormat = SelectSurfaceFormat(surfaceFormats);
	auto newPresentMode = SelectPresentMode(presentModes);
	auto windowExtent = platform->GetExtent();

	extent = SelectExtent(surfaceCapabilities, windowExtent.width, windowExtent.height);
	surfaceFormat = newSurfaceFormat;
	presentMode = newPresentMode;
}

void cp::Swapchain::Create(vk::RenderPass _mainRenderPass)
{
	if(!mainRenderPass) mainRenderPass = _mainRenderPass;
	CreateData();
	maxFramesInFlight = static_cast<uint32>(frames.size());
	currentFrame = 0;
}

void cp::Swapchain::Recreate()
{
	auto windowExtent = platform->GetExtent();

	while (windowExtent.width == 0 || windowExtent.height == 0)
	{
		extent = windowExtent;
		switch (platform->GetType())
		{
		case cp::PlatformType::GLFW:
			glfwWaitEvents();
			break;
		case cp::PlatformType::QT:
			QCoreApplication::processEvents();
			break;
		default:
			break;
		}
	}

	context->GetDevice().waitIdle();

	Cleanup();
	Setup();
	Create(mainRenderPass);
}

void cp::Swapchain::Cleanup()
{
	context->GetDevice().destroySwapchainKHR(swapchain);

	for (auto frame : frames)
	{
		delete frame;
	}
}

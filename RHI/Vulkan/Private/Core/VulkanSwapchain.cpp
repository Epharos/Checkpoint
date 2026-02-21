#include "../pch.hpp"

#include "VulkanSwapchain.hpp"

namespace cp
{
	cp::VulkanSwapchain::VulkanSwapchain(ILogger& _logger, const SwapchainInfo& _info)
		: ISwapchain(_logger, _info)
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
		
	}

	void VulkanSwapchain::Cleanup()
	{
	
	}
}
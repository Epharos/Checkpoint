#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"

namespace Render
{
	class Frame;

	class Swapchain
	{
	private:
		Context::VulkanContext* context;

		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		std::vector<vk::PresentModeKHR> presentModes;

		vk::SwapchainKHR swapchain;
		vk::Format format;
		vk::Extent2D extent;

		std::vector<Frame*> frames;

		uint32 currentFrame;
		uint32 maxFramesInFlight;

		void CreateData();

		void QuerySupport();

		vk::SurfaceFormatKHR SelectSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& _formats, vk::Format _format = vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR _colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear);
		vk::PresentModeKHR SelectPresentMode(const std::vector<vk::PresentModeKHR>& _presentModes, vk::PresentModeKHR _presentMode = vk::PresentModeKHR::eMailbox);
		vk::Extent2D SelectExtent(const vk::SurfaceCapabilitiesKHR& _capabilities, const uint32& _width, const uint32& _height);



	public:
		Swapchain(Context::VulkanContext* _context);
		~Swapchain();

		void Create();
		void Recreate();
		void Cleanup();
	};
}
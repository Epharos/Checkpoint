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
		vk::SurfaceFormatKHR surfaceFormat;
		vk::PresentModeKHR presentMode;
		vk::Extent2D extent;

		std::vector<Frame*> frames;
		vk::RenderPass mainRenderPass;

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

		void Setup();

		void Create(vk::RenderPass _mainRenderPass);
		void Recreate();
		void Cleanup();

		inline constexpr vk::SwapchainKHR& GetSwapchain() { return swapchain; }
		inline constexpr vk::PresentModeKHR& GetPresentMode() { return presentMode; }
		inline constexpr vk::SurfaceFormatKHR& GetSurfaceFormat() { return surfaceFormat; }
		inline constexpr vk::Format& GetFormat() { return surfaceFormat.format; }
		inline constexpr vk::Extent2D& GetExtent() { return extent; }
		inline constexpr std::vector<Frame*>& GetFrames() { return frames; }
		inline constexpr Frame* GetCurrentFrame() { return frames[currentFrame]; }
		inline constexpr uint32 GetCurrentFrameIndex() { return currentFrame; }
		inline constexpr uint32 GetMaxFramesInFlight() { return maxFramesInFlight; }
		inline constexpr uint32 GetFrameCount() { return static_cast<uint32>(frames.size()); }
		inline constexpr vk::SurfaceCapabilitiesKHR& GetSurfaceCapabilities() { return surfaceCapabilities; }
		inline constexpr std::vector<vk::SurfaceFormatKHR>& GetSurfaceFormats() { return surfaceFormats; }
		inline constexpr std::vector<vk::PresentModeKHR>& GetPresentModes() { return presentModes; }
		inline constexpr vk::RenderPass& GetMainRenderPass() { return mainRenderPass; }
	};
}
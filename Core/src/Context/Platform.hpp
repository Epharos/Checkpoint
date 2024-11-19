#pragma once

#include "../pch.hpp"

typedef GLFWwindow* Window; //future proof, in case we change the windowing library

namespace Context
{
	struct VulkanContextInfo;

	class Platform
	{
	private:
		Window window;

		vk::Extent2D extent;

		static void FramebufferResizeCallback(Window _window, int _width, int _height);
	public:
		void Initialize(VulkanContextInfo _context, vk::Extent2D _extent = vk::Extent2D(720, 480));
		bool ShouldClose() const;
		void PollEvents() const;
		void CleanUp() const;

		vk::Extent2D GetFrameBufferExtent() const;

		inline constexpr Window GetWindow() const { return window; }
		inline constexpr vk::Extent2D GetExtent() const { return extent; }
	};
}
#include "pch.hpp"

#include "Platform.hpp"
#include "VulkanContext.hpp"

namespace Context
{
	void Platform::FramebufferResizeCallback(Window _window, int _width, int _height)
	{
		auto platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(_window));
		platform->extent = vk::Extent2D(_width, _height);
	}

	void Platform::Initialize(VulkanContextInfo _context, vk::Extent2D _extent)
	{
		if (!glfwInit())
			LOG_ERROR("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		extent = _extent;

		if (window = glfwCreateWindow(_extent.width, _extent.height, _context.appName.c_str(), nullptr, nullptr))
		{
			glfwSetWindowUserPointer(window, this);
			glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
		}
	}

	bool Platform::ShouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	void Platform::PollEvents() const
	{
		glfwPollEvents();
	}

	void Platform::CleanUp() const
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	vk::Extent2D Platform::GetFrameBufferExtent() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		return vk::Extent2D(width, height);
	}
}

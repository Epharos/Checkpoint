#include "pch.hpp"

#include "PlatformGLFW.hpp"
#include "../VulkanContext.hpp"

namespace Context
{
	void PlatformGLFW::Initialize(VulkanContextInfo _context, vk::Extent2D _extent)
	{
		if (!glfwInit())
			LOG_ERROR("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		if (window = glfwCreateWindow(_extent.width, _extent.height, _context.appName.c_str(), nullptr, nullptr))
		{
			glfwSetWindowUserPointer(window, this);
		}
	}

	bool PlatformGLFW::ShouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	void PlatformGLFW::PollEvents() const
	{
		glfwPollEvents();
	}

	void PlatformGLFW::CleanUp() const
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void PlatformGLFW::SetTitle(const std::string& _title) const
	{
		glfwSetWindowTitle(window, _title.c_str());
	}

	vk::Extent2D PlatformGLFW::GetExtent() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		return vk::Extent2D(width, height);
	}
}

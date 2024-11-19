#include "pch.hpp"

#include "Platform.hpp"
#include "VulkanContext.hpp"

namespace Context
{
	void Platform::Initialize(VulkanContextInfo _context)
	{
		if (!glfwInit())
			LOG_ERROR("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(800, 600, _context.appName.c_str(), nullptr, nullptr);
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
}

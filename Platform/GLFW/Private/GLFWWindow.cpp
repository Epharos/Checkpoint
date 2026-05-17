#include "GLFWWindow.hpp"

#include <../../../Common/Public/Common/Core/Macros.hpp>

#if defined(CP_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

namespace cp
{
	GLFWWindow::GLFWWindow(const WindowInfo& _info) : IWindow(_info)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, _info.resizable ? GLFW_TRUE : GLFW_FALSE);

		window = glfwCreateWindow(_info.extent.x(), _info.extent.y(), _info.title.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* _win, int _width, int _height)
		{
			if (_width > 0 && _height > 0)
			{
				auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(_win));

				if (self != nullptr)
				{
					self->InvokeResizeCallback(Extent2D<int>{ _width, _height });
				}
			}
		});
	}

	GLFWWindow::~GLFWWindow()
	{
		CleanUp();
	}

	void* GLFWWindow::GetNativeWindowHandle() const
	{
		return glfwGetWin32Window(window);
	}

	float GLFWWindow::GetAspectRatio() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return static_cast<float>(width) / static_cast<float>(height);
	}

	void GLFWWindow::PollEvents() const
	{
		glfwPollEvents();
	}

	void GLFWWindow::WaitForEvents() const
	{
		glfwWaitEvents();
	}

	bool GLFWWindow::ShouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	void GLFWWindow::SetTitle(std::string_view _title) const
	{
		glfwSetWindowTitle(window, _title.data());
	}

	std::string_view GLFWWindow::GetTitle() const
	{
		return std::string_view(glfwGetWindowTitle(window));
	}

	void GLFWWindow::SetExtent(Extent2D<int> _extent)
	{
		glfwSetWindowSize(window, _extent.x(), _extent.y());
	}

	Extent2D<int> GLFWWindow::GetExtent() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		
		return Extent2D<int>{ width, height };
	}

	void GLFWWindow::SetResizable(bool _resizable)
	{
		glfwSetWindowAttrib(window, GLFW_RESIZABLE, _resizable ? GLFW_TRUE : GLFW_FALSE);
	}

	bool GLFWWindow::IsResizable() const
	{
		return glfwGetWindowAttrib(window, GLFW_RESIZABLE) == GLFW_TRUE;
	}

	void GLFWWindow::SetFullscreen(bool _fullscreen)
	{
		glfwSetWindowMonitor(window, _fullscreen ? glfwGetPrimaryMonitor() : nullptr, 0, 0, 0, 0, GLFW_DONT_CARE);
	}

	bool GLFWWindow::IsFullscreen() const
	{
		return glfwGetWindowMonitor(window) != nullptr;
	}

	void GLFWWindow::SetVSyncActive(bool _vsync)
	{
		
	}

	bool GLFWWindow::IsVSyncActive() const
	{
		return false;
	}

	void GLFWWindow::CleanUp() const
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
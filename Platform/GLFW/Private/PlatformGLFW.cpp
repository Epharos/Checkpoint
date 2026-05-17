#include <Platform/PlatformAPI.hpp>

#include "GLFWWindow.hpp"

static cp::IWindow* CreateGLFWWindow(const cp::WindowInfo& _info)
{
	return new cp::GLFWWindow(_info);
}

static void DestroyGLFWWindow(cp::IWindow* _window)
{
	delete _window;
}

CP_DECLARE_PLATFORM("GLFW", CreateGLFWWindow, DestroyGLFWWindow)

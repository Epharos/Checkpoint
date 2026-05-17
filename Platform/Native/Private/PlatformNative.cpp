#include <Platform/PlatformAPI.hpp>

#include "NativeWindow.hpp"

static cp::IWindow* CreateNativeWindow(const cp::WindowInfo& _info)
{
	return new cp::NativeWindow(_info);
}

static void DestroyNativeWindow(cp::IWindow* _window)
{
	delete _window;
}

CP_DECLARE_PLATFORM("Native", CreateNativeWindow, DestroyNativeWindow)

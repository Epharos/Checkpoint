#include "../pch.hpp"

#include "Surface.hpp"

#include "Instance.hpp"

#include <Profiling.hpp>
#include <Log.hpp>

namespace cp
{
	Surface::Surface(ILogger& _logger, const SurfaceInfo& _info, Instance& _instance)
		: logger(_logger), info(_info), instance(_instance)
	{
		Initialize();
	}

	Surface::~Surface()
	{
		Cleanup();
	}

	void Surface::Initialize()
	{
		CP_EXPECT_MSG(info.nativeWindowHandle != nullptr, "Native window handle is null, cannot create Vulkan surface");

		{
			CP_PROFILE_SCOPE("VulkanSurface#Initialize");

#if defined(CP_PLATFORM_WINDOWS)
			vk::Win32SurfaceCreateInfoKHR sci;

			if (info.nativeWindowHandle == nullptr)
			{
				logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Native window handle is null, cannot create Vulkan surface")));
				return;
			}

			sci.hwnd = (HWND)info.nativeWindowHandle;
			sci.hinstance = GetModuleHandle(nullptr);

			surface = instance.GetHandle().createWin32SurfaceKHR(sci);
#else
			// TODO: Implement surface creation for other platforms (e.g., Xlib, Wayland, etc.)
			#error "Surface creation not implemented for this platform"
#endif
		}

		if (surface == VK_NULL_HANDLE)
		{
			logger.Log(CP_LOG_EVENT(cp::ILogger::Critical, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Failed to create Vulkan surface")));
			return;
		}

		logger.Log(CP_LOG_EVENT(cp::ILogger::Info, VulkanRHI_Label, cp::Message::Create<cp::TextComponent>("Vulkan surface created successfully")));
	}

	void Surface::Cleanup()
	{
		if (surface != VK_NULL_HANDLE)
		{
			instance.GetHandle().destroySurfaceKHR(surface);
		}
	}
}
#include "../pch.hpp"

#include "VulkanSurface.hpp"

#include "VulkanInstance.hpp"

#include <Profiling.hpp>
#include <Log.hpp>

namespace cp
{
	VulkanSurface::VulkanSurface(ILogger& _logger, const SurfaceInfo& _info, VulkanInstance& _instance)
		: logger(_logger), info(_info), instance(_instance)
	{
		Initialize();
	}

	VulkanSurface::~VulkanSurface()
	{
		Cleanup();
	}

	void VulkanSurface::Initialize()
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

	void VulkanSurface::Cleanup()
	{
		if (surface != VK_NULL_HANDLE)
		{
			instance.GetHandle().destroySurfaceKHR(surface);
		}
	}
}
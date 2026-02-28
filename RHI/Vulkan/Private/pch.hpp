#pragma once

#include <memory>

#include <Macros.hpp>

#if defined(CP_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.hpp>

#define CP_VK_CHECK(op) \
{ \
	auto result = (op); \
	CP_ENSURE_MSG(result == vk::Result::eSuccess, ("Vulkan error: " + static_cast<size_t>(result))); \
}

constinit static const char* VulkanRHI_Label = "RHI (Vulkan)";
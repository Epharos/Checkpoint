#pragma once

#include <Macros.hpp>

#if defined(CP_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.hpp>

constinit static const char* VulkanRHI_Label = "RHI Vulkan";
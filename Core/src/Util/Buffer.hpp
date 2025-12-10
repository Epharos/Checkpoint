#pragma once

#include "../pch.hpp"

namespace cp
{
	struct Buffer
	{
		vk::Buffer buffer = VK_NULL_HANDLE;
		vk::DeviceMemory memory = VK_NULL_HANDLE;
	};
}
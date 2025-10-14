#pragma once

#include "../pch.hpp"

namespace cp
{
	struct Buffer
	{
		vk::Buffer buffer;
		vk::DeviceMemory memory;
	};
}
#pragma once

#include "../pch.hpp"

namespace cp
{
	uint32_t FindMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
}
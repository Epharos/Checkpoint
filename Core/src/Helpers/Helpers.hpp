#pragma once

#include "../pch.hpp"

namespace Helper
{
	namespace Memory
	{
		uint32_t FindMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		vk::Buffer CreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::DeviceMemory& bufferMemory);
		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, void* data);
		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, const void* data);
		void CopyBuffer(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size);
		void DestroyBuffer(const vk::Device& device, const vk::Buffer& buffer, const vk::DeviceMemory& bufferMemory);
	}

	namespace Format
	{
		vk::Format FindSupportedFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format FindDepthFormat(const vk::PhysicalDevice& physicalDevice);
	}

	namespace File
	{
		std::vector<char> ReadShaderFile(const std::string& _path);
	}
}
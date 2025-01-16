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

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, vk::DeviceSize offset, void* data);
		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, vk::DeviceSize offset, const void* data);
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

	namespace Image
	{
		void CreateImage(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
		void TransitionImageLayout(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void CopyBufferToImage(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height);
		void CreateImageView(const vk::Device& device, const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView);
		void CreateSampler(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::Sampler& sampler);
	}

	namespace CommandBuffer
	{
		vk::CommandBuffer BeginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool);
		void EndSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::CommandBuffer& commandBuffer);
	}
}
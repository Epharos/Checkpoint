#include "pch.hpp"

#include "Helpers.hpp"

namespace Helper
{
	namespace Memory
	{
		uint32_t FindMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
		{
			vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			throw std::runtime_error("Failed to find suitable memory type!");
		}

		vk::Buffer CreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::DeviceMemory& bufferMemory)
		{
			vk::BufferCreateInfo bufferCreateInfo;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = usage;
			bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

			vk::Buffer buffer = device.createBuffer(bufferCreateInfo);

			vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);

			vk::MemoryAllocateInfo memoryAllocateInfo;
			memoryAllocateInfo.allocationSize = memoryRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties);

			bufferMemory = device.allocateMemory(memoryAllocateInfo);
			device.bindBufferMemory(buffer, bufferMemory, 0);

			return buffer;
		}
	}

	namespace Format
	{
		vk::Format FindSupportedFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
		{
			for (vk::Format format : candidates)
			{
				vk::FormatProperties props = physicalDevice.getFormatProperties(format);

				if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
				{
					return format;
				}
				else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
				{
					return format;
				}
			}

			throw std::runtime_error("Failed to find supported format!");
		}

		vk::Format FindDepthFormat(const vk::PhysicalDevice& physicalDevice)
		{
			return FindSupportedFormat(physicalDevice, { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		}
	}
}
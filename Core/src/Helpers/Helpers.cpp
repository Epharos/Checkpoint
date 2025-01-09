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

		void MapMemory(const vk::Device& device, const vk::DeviceMemory& memory, vk::DeviceSize size, void* data)
		{
			void* mappedMemory = device.mapMemory(memory, 0, size);
			memcpy(mappedMemory, data, size);
			device.unmapMemory(memory);
		}

		void CopyBuffer(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& queue, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size)
		{
			vk::CommandBufferAllocateInfo allocInfo;
			allocInfo.level = vk::CommandBufferLevel::ePrimary;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

			vk::CommandBufferBeginInfo beginInfo;
			beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

			commandBuffer.begin(beginInfo);

			vk::BufferCopy copyRegion;
			copyRegion.size = size;
			commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

			commandBuffer.end();

			vk::SubmitInfo submitInfo;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			queue.submit(1, &submitInfo, nullptr);
			queue.waitIdle();

			device.freeCommandBuffers(commandPool, commandBuffer);
		}

		void DestroyBuffer(const vk::Device& device, const vk::Buffer& buffer, const vk::DeviceMemory& bufferMemory)
		{
			device.destroyBuffer(buffer);
			device.freeMemory(bufferMemory);
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

	namespace File
	{
		std::vector<char> ReadShaderFile(const std::string& _path)
		{
			std::ifstream stream(_path, std::ios::ate | std::ios::binary);

			if (!stream.is_open())
			{
				throw std::runtime_error("Failed to open file: " + _path);
			}

			size_t fileSize = static_cast<size_t>(stream.tellg());

			std::vector<char> buffer(fileSize);
			stream.seekg(0);
			stream.read(buffer.data(), fileSize);

			stream.close();

			return buffer;
		}
	}
}
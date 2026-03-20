#include "../pch.hpp"

#include "Buffer.hpp"

#include "../Core/Device.hpp"
#include "../Core/PhysicalDevice.hpp"
#include "../Utilities/MemoryHelper.hpp"
#include "../Utilities/VulkanConverter.hpp"

namespace cp
{
	namespace
	{
		vk::MemoryPropertyFlags ResolveMemoryFlags(const BufferInfo& _info)
		{
			if (_info.cpuVisible)
			{
				return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			}

			return vk::MemoryPropertyFlagBits::eDeviceLocal;
		}
	}

	Buffer::Buffer(ILogger& _logger, const BufferInfo& _info, Device& _device)
		: IBuffer(_info), device(_device), logger(_logger)
	{
		Initialize();
	}

	Buffer::~Buffer()
	{
		Cleanup();
	}

	void* Buffer::Map()
	{
		CP_EXPECT_MSG(info.cpuVisible, "Cannot map a non cpu-visible buffer");
		CP_EXPECT_MSG(mappedMemory == nullptr, "Buffer is already mapped");

		mappedMemory = device.GetHandle().mapMemory(memory, 0, info.sizeBytes);
		CP_ENSURE_MSG(mappedMemory, "Could not map buffer memory");

		return mappedMemory;
	}

	void Buffer::Unmap()
	{
		CP_EXPECT_MSG(mappedMemory != nullptr, "Buffer is not mapped");

		device.GetHandle().unmapMemory(memory);
		mappedMemory = nullptr;
	}

	void Buffer::Initialize()
	{
		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.setSize(info.sizeBytes);
		bufferCreateInfo.setUsage(EnumBitsCast<vk::BufferUsageFlags>(info.usage));
		bufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);

		CP_VK_CHECK(device.GetHandle().createBuffer(&bufferCreateInfo, nullptr, &buffer));
		CP_ENSURE_MSG(buffer != VK_NULL_HANDLE, "Buffer was not initialized");

		const vk::MemoryRequirements memoryRequirements = device.GetHandle().getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.setAllocationSize(memoryRequirements.size);
		memoryAllocateInfo.setMemoryTypeIndex(
			FindMemoryType(
				device.GetPhysicalDevice().GetHandle(),
				memoryRequirements.memoryTypeBits,
				ResolveMemoryFlags(info)
			)
		);

		memory = device.GetHandle().allocateMemory(memoryAllocateInfo);
		CP_ENSURE_MSG(memory != VK_NULL_HANDLE, "Buffer memory was not initialized");

		device.GetHandle().bindBufferMemory(buffer, memory, 0);
	}

	void Buffer::Cleanup() const
	{
		if (memory != VK_NULL_HANDLE)
		{
			device.GetHandle().freeMemory(memory);
		}

		if (buffer != VK_NULL_HANDLE)
		{
			device.GetHandle().destroyBuffer(buffer);
		}
	}
}

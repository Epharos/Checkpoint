#include "CommandBuffer.hpp"

#include "Device.hpp"

namespace cp
{
    CommandBuffer::CommandBuffer(Device &_device, QueueType _queueType, uint32_t _queueIndex)
        : device(_device), commandPool(device.GetCommandPool(_queueType, _queueIndex))
    {
        Initialize();
    }

    CommandBuffer::~CommandBuffer()
    {
        Cleanup();
    }

    void CommandBuffer::Begin() const
    {
        commandBuffer.reset(); // TODO : Should I reset here or make it its own method ?

        const vk::CommandBufferBeginInfo beginInfo;
        commandBuffer.begin(beginInfo);
    }

    void CommandBuffer::End() const
    {
        commandBuffer.end();
    }

    void CommandBuffer::Initialize()
    {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.setCommandPool(commandPool);
        commandBufferAllocateInfo.setCommandBufferCount(1);
        commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        commandBuffer = device.GetHandle().allocateCommandBuffers(commandBufferAllocateInfo)[0];
    }

    void CommandBuffer::Cleanup() const
    {
        device.GetHandle().freeCommandBuffers(commandPool, commandBuffer);
    }
}

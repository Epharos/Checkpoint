#include "../pch.hpp"

#include "CommandAllocator.hpp"

#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "../../../../Common/Private/Assert.hpp"

namespace cp
{
    CommandAllocator::CommandAllocator(IQueue& _queue, Device& _device)
        : ICommandAllocator(_queue), device(_device), queue(reinterpret_cast<Queue&>(_queue))
    {
        Initialize();
    }

    CommandAllocator::~CommandAllocator()
    {
        Cleanup();
    }

    std::unique_ptr<ICommandBuffer> CommandAllocator::Allocate()
    {
        return std::make_unique<CommandBuffer>(*this);
    }

    void CommandAllocator::Reset()
    {
        CP_EXPECT_MSG(commandPool, "Failed to reset command pool");

        device.GetHandle().resetCommandPool(commandPool);
    }

    void CommandAllocator::Initialize()
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.setQueueFamilyIndex(queue.GetFamilyIndex());

        commandPool = device.GetHandle().createCommandPool(commandPoolCreateInfo);

        CP_ENSURE_MSG(commandPool, "Failed to create command pool");
    }

    void CommandAllocator::Cleanup() const
    {
        if (commandPool)
        {
            device.GetHandle().destroyCommandPool(commandPool);
        }
    }
}

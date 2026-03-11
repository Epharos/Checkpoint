#pragma once

#include "../pch.hpp"

#include <RHI/Core.hpp>

namespace cp
{
    class Queue;
    class Device;

    class CommandAllocator final : public ICommandAllocator
    {
    public:
        CommandAllocator(IQueue& _queue, Device& _device);
        ~CommandAllocator() override;

    public:
        [[nodiscard]] vk::CommandPool& GetHandle() { return commandPool; }
        [[nodiscard]] const vk::CommandPool& GetHandle() const { return commandPool; }

        [[nodiscard]] Device& GetDevice() { return device; }
        [[nodiscard]] const Device& GetDevice() const { return device; }

        [[nodiscard]] Queue& GetQueue() { return queue; }
        [[nodiscard]] const Queue& GetQueue() const { return queue; }

    public:
        std::unique_ptr<ICommandBuffer> Allocate() override;

        void Reset() override;

    private:
        void Initialize();
        void Cleanup() const;

    private:
        Device& device;
        Queue& queue;

        vk::CommandPool commandPool { VK_NULL_HANDLE };
    };
}
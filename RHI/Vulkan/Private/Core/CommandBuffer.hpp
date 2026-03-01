#pragma once

#include <RHI/Core.hpp>

namespace cp
{
    class Device;

    class CommandBuffer final : public ICommandBuffer
    {
    public:
        CommandBuffer(Device& _device, QueueType _queueType, uint32_t _queueIndex);
        ~CommandBuffer() override;

    public:
        [[nodiscard]] vk::CommandBuffer& GetHandle() { return commandBuffer; }
        [[nodiscard]] const vk::CommandBuffer& GetHandle() const { return commandBuffer; }

    public:
        void Begin() const override;
        void End() const override;

    public:
        void Initialize();
        void Cleanup() const;

    private:
        Device& device;
        vk::CommandPool& commandPool;

        vk::CommandBuffer commandBuffer { VK_NULL_HANDLE };
    };
}

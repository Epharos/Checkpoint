#pragma once

#include "../pch.hpp"

#include <RHI/Synchro.hpp>

namespace cp
{
    class Device;

    class TimelineSemaphore final : public ITimelineSemaphore
    {
    public:
        TimelineSemaphore(Device& _device);
        ~TimelineSemaphore() override;
    public:
        void SignalCPU(size_t _value) const override;
        void WaitCPU(size_t _value) const override;
        [[nodiscard]] size_t GetCompletedValue() const override;

        /**
        * @brief Returns the underlying Vulkan semaphore handle.
        *        Required when passing this semaphore to Vulkan API calls directly (e.g. bridge submits).
        *
        * @return The Vulkan semaphore handle.
        */
        [[nodiscard]] vk::Semaphore& GetHandle() { return semaphore; }
        [[nodiscard]] const vk::Semaphore& GetHandle() const { return semaphore; }

    private:
        void Initialize();
        void Cleanup() const;

    private:
        vk::Semaphore semaphore;

        Device& device;
    };
}
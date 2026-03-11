#pragma once

#include <cstdlib>

namespace cp
{
    class IQueue;

    class ITimelineSemaphore
    {
    public:
        virtual ~ITimelineSemaphore() = default;

        /**
        * @brief Signals the semaphore from the CPU to the given value.
        *        The value must be strictly greater than the semaphore's current value.
        *
        * @param _value The value to signal.
        */
        virtual void SignalCPU(size_t _value) const = 0;

        /**
        * @brief Blocks the CPU until the semaphore reaches or exceeds the given value.
        *
        * @param _value The value to wait for.
        */
        virtual void WaitCPU(size_t _value) const = 0;

        /**
        * @brief Returns the last value that has been signaled and is visible to the CPU.
        *        This reflects completed GPU work — it may lag behind pending signals still in-flight on the GPU.
        *
        * @return The last completed semaphore value.
        */
        [[nodiscard]] virtual size_t GetCompletedValue() const = 0;
    };
}

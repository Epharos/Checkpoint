#pragma once

#include <cstdlib>

namespace cp
{
    class IQueue;

    class ITimelineSemaphore
    {
    public:
        virtual ~ITimelineSemaphore() = default;

        virtual void SignalCPU(size_t _value) const = 0;
        virtual void WaitCPU(size_t _value) const = 0;

        [[nodiscard]] virtual size_t GetCompletedValue() const = 0;
    };
}

#include "../pch.hpp"

#include "TimelineSemaphore.hpp"

#include "Common/Core/Assert.hpp"
#include "../Core/Device.hpp"
#include "../Core/Queue.hpp"

namespace cp
{
    TimelineSemaphore::TimelineSemaphore(Device &_device)
        : device(_device)
    {
        Initialize();
    }

    TimelineSemaphore::~TimelineSemaphore()
    {
        Cleanup();
    }

    void TimelineSemaphore::SignalCPU(const size_t _value) const
    {
        CP_EXPECT_MSG(semaphore, "Could not signal using a null semaphore");

        vk::SemaphoreSignalInfo info;
        info.setSemaphore(semaphore);
        info.setValue(_value);

        device.GetHandle().signalSemaphore(info);
    }

    void TimelineSemaphore::WaitCPU(const size_t _value) const
    {
        CP_EXPECT_MSG(semaphore, "Could not wait using a null semaphore");

        vk::SemaphoreWaitInfo info;
        info.setSemaphoreCount(1);
        info.setPSemaphores(&semaphore);
        info.setPValues(&_value);

        CP_VK_CHECK(device.GetHandle().waitSemaphores(info, std::numeric_limits<size_t>::max()));
    }

    size_t TimelineSemaphore::GetCompletedValue() const
    {
        CP_EXPECT_MSG(semaphore, "Could not get the value of a null semaphore");

        return device.GetHandle().getSemaphoreCounterValue(semaphore);
    }

    void TimelineSemaphore::Initialize()
    {
        vk::SemaphoreTypeCreateInfo semaphoreTypeCreateInfo;
        semaphoreTypeCreateInfo.setSemaphoreType(vk::SemaphoreType::eTimeline);
        semaphoreTypeCreateInfo.setInitialValue(0);

        vk::SemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.pNext = &semaphoreTypeCreateInfo;

        semaphore = device.GetHandle().createSemaphore(semaphoreCreateInfo);

        CP_ENSURE_MSG(semaphore, "Could not create semaphore");
    }

    void TimelineSemaphore::Cleanup() const
    {
        if (semaphore != VK_NULL_HANDLE)
        {
            device.GetHandle().destroySemaphore(semaphore);
        }
    }
}

#pragma once

#include <cstdint>
#include <mutex>

namespace cp
{
    template<typename T, size_t Capacity>
    class ThreadSafeStaticRingBuffer
    {
    public:
        bool PushBack(const T& item)
        {
            std::lock_guard lock(mutex);
            if (
                const size_t next = (head + 1) % Capacity;
                next != tail
            )
            {
                data[head] = item;
                head = next;
                return true;
            }

            return false;
        }

        bool PopFront(T& item)
        {
            std::lock_guard lock(mutex);

            if (tail != head)
            {
                item = data[tail];
                tail = (tail + 1) % Capacity;
                return true;
            }

            return false;
        }

        [[nodiscard]] bool IsEmpty() const
        {
            return tail == head;
        }

    private:
        T data[Capacity]; // TODO : Use StaticAllocator when implemented
        size_t head = 0;
        size_t tail = 0;
        std::mutex mutex;
    };
}

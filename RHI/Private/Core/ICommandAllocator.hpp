#pragma once

#include <memory>

namespace cp
{
    class ICommandBuffer;
    class IQueue;

    class ICommandAllocator
    {
    public:
        ICommandAllocator(IQueue& _queue) {}
        virtual ~ICommandAllocator() = default;

        /**
        * @brief Allocates a new command buffer from this command pool.
        *
        * @return A unique pointer to the newly allocated command buffer.
        */
        virtual std::unique_ptr<ICommandBuffer> Allocate() = 0;

        /**
        * @brief Resets all command buffers allocated from this pool, allowing them to be re-recorded.
        *        The GPU must have finished executing all commands from this pool before calling Reset.
        */
        virtual void Reset() = 0;
    };
}
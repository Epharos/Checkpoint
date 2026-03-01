#pragma once

namespace cp
{
    class ICommandBuffer
    {
    public:
        virtual ~ICommandBuffer() = default;

        virtual void Begin() const = 0;
        virtual void End() const = 0;
    };

    using ICommandList = ICommandBuffer;
}

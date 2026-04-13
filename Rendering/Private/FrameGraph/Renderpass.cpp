#include "Renderpass.hpp"
#include "../Renderer.hpp"
#include "../../../RHI/Private/Core/IQueue.hpp"

namespace cp
{
    ICommandBuffer& RenderPassExecutionContext::GetGraphicsBuffer(const uint32_t _index) const
    {
        return frameContext.GetCommandBuffer(QueueType::Graphics, _index);
    }

    ICommandBuffer& RenderPassExecutionContext::GetComputeBuffer(const uint32_t _index) const
    {
        return frameContext.GetCommandBuffer(QueueType::Compute, _index);
    }

    ICommandBuffer& RenderPassExecutionContext::GetCopyBuffer(const uint32_t _index) const
    {
        return frameContext.GetCommandBuffer(QueueType::Transfer, _index);
    }
}

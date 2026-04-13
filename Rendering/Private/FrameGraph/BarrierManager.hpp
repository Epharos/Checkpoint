#pragma once

#include <unordered_map>
#include <vector>

#include <RHI/Synchro.hpp>

#include "FrameGraphBuilder.hpp"

namespace cp
{
    class IRenderPass;

    class BarrierManager
    {
    public:
        BarrierManager() = default;
        ~BarrierManager() = default;

        void BuildPassBarriers(
            const std::vector<IRenderPass*>& _executionOrder,
            const std::vector<FrameGraphBuilder::TextureResourceAccess>& _textureAccesses,
            const std::vector<FrameGraphBuilder::BufferResourceAccess>& _bufferAccesses,
            const std::unordered_map<FramegraphResource*, FrameGraphBuilder::TextureSynchronization>& _textureFinalStates,
            const std::unordered_map<FramegraphResource*, FrameGraphBuilder::BufferSynchronization>& _bufferFinalStates
        );

        [[nodiscard]] const std::vector<IBarrier>& GetPassBeginBarriers(IRenderPass* _pass) const;
        [[nodiscard]] const std::vector<IBarrier>& GetPassEndBarriers(IRenderPass* _pass) const;

        void Clear();

    private:
        std::unordered_map<IRenderPass*, std::vector<IBarrier>> beginBarriersByPass;
        std::unordered_map<IRenderPass*, std::vector<IBarrier>> endBarriersByPass;
    };
}

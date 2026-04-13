#include "BarrierManager.hpp"

#include <unordered_map>

#include <Common/Core/Assert.hpp>

#include "FramegraphResource.hpp"
#include "Renderpass.hpp"

namespace cp
{
    namespace
    {
        using TextureAccessByPass = std::unordered_map<IRenderPass*, std::vector<const FrameGraphBuilder::TextureResourceAccess*>>;
        using BufferAccessByPass = std::unordered_map<IRenderPass*, std::vector<const FrameGraphBuilder::BufferResourceAccess*>>;

        struct LastTextureAccessState
        {
            IRenderPass* pass = nullptr;
            FrameGraphBuilder::TextureSynchronization sync;
            bool isWrite = false;
        };

        struct LastBufferAccessState
        {
            IRenderPass* pass = nullptr;
            FrameGraphBuilder::BufferSynchronization sync;
            bool isWrite = false;
        };

        const std::vector<IBarrier>& EmptyBarriers()
        {
            static constexpr std::vector<IBarrier> emptyBarriers;
            return emptyBarriers;
        }

        TextureAccessByPass BuildTextureAccessesByPass(
            const std::vector<IRenderPass*>& _executionOrder,
            const std::vector<FrameGraphBuilder::TextureResourceAccess>& _textureAccesses
        )
        {
            TextureAccessByPass textureAccessesByPass;
            textureAccessesByPass.reserve(_executionOrder.size());

            for (const auto& access : _textureAccesses)
            {
                textureAccessesByPass[access.pass].push_back(&access);
            }

            return textureAccessesByPass;
        }

        BufferAccessByPass BuildBufferAccessesByPass(
            const std::vector<IRenderPass*>& _executionOrder,
            const std::vector<FrameGraphBuilder::BufferResourceAccess>& _bufferAccesses
        )
        {
            BufferAccessByPass bufferAccessesByPass;
            bufferAccessesByPass.reserve(_executionOrder.size());

            for (const auto& access : _bufferAccesses)
            {
                bufferAccessesByPass[access.pass].push_back(&access);
            }

            return bufferAccessesByPass;
        }

        void BuildPassTextureBeginBarriers(
            IRenderPass* _pass,
            const TextureAccessByPass& _textureAccessesByPass,
            std::unordered_map<IRenderPass*, std::vector<IBarrier>>& _beginBarriersByPass,
            std::unordered_map<FramegraphResource*, LastTextureAccessState>& _lastTextureAccessByResource
        )
        {
            const auto textureAccessesIt = _textureAccessesByPass.find(_pass);
            if (textureAccessesIt == _textureAccessesByPass.end())
            {
                return;
            }

            auto& passBeginBarriers = _beginBarriersByPass[_pass];

            for (const auto* access : textureAccessesIt->second)
            {
                ITexture* texture = access->resource->GetTexture();
                CP_ASSERT_MSG(texture != nullptr, "Texture resource is not allocated while building barriers");

                const auto lastAccessIt = _lastTextureAccessByResource.find(access->resource);

                if (lastAccessIt == _lastTextureAccessByResource.end())
                {
                    const TextureBarrierInfo initialBarrierInfo {
                        .texture = *texture,
                        .srcLayout = texture->GetLayout(),
                        .dstLayout = access->sync.layout,
                        .srcStage = PipelineStage::Top,
                        .dstStage = access->sync.stage,
                        .srcAccess = Access::None,
                        .dstAccess = access->sync.access,
                        .baseMip = access->sync.baseMip,
                        .mipCount = access->sync.mipCount,
                        .baseLayer = access->sync.baseLayer,
                        .layerCount = access->sync.layerCount
                    };

                    passBeginBarriers.emplace_back(initialBarrierInfo);
                }
                else
                {
                    const bool hasWriteHazard = lastAccessIt->second.isWrite || access->isWrite;

                    if (hasWriteHazard)
                    {
                        const TextureBarrierInfo barrierInfo {
                            .texture = *texture,
                            .srcLayout = lastAccessIt->second.sync.layout,
                            .dstLayout = access->sync.layout,
                            .srcStage = lastAccessIt->second.sync.stage,
                            .dstStage = access->sync.stage,
                            .srcAccess = lastAccessIt->second.sync.access,
                            .dstAccess = access->sync.access,
                            .baseMip = access->sync.baseMip,
                            .mipCount = access->sync.mipCount,
                            .baseLayer = access->sync.baseLayer,
                            .layerCount = access->sync.layerCount
                        };

                        passBeginBarriers.emplace_back(barrierInfo);
                    }
                }

                _lastTextureAccessByResource[access->resource] = {
                    .pass = _pass,
                    .sync = access->sync,
                    .isWrite = access->isWrite
                };
            }
        }

        void BuildPassBufferBeginBarriers(
            IRenderPass* _pass,
            const BufferAccessByPass& _bufferAccessesByPass,
            std::unordered_map<IRenderPass*, std::vector<IBarrier>>& _beginBarriersByPass,
            std::unordered_map<FramegraphResource*, LastBufferAccessState>& _lastBufferAccessByResource
        )
        {
            const auto bufferAccessesIt = _bufferAccessesByPass.find(_pass);
            if (bufferAccessesIt == _bufferAccessesByPass.end())
            {
                return;
            }

            auto& passBeginBarriers = _beginBarriersByPass[_pass];
            for (const auto* access : bufferAccessesIt->second)
            {
                IBuffer* buffer = access->resource->GetBuffer();
                CP_EXPECT_MSG(buffer != nullptr, "Buffer resource is not allocated while building barriers");

                const auto lastAccessIt = _lastBufferAccessByResource.find(access->resource);

                if (lastAccessIt == _lastBufferAccessByResource.end())
                {
                    const BufferBarrierInfo initialBarrierInfo {
                        .buffer = *buffer,
                        .srcStage = PipelineStage::Top,
                        .dstStage = access->sync.stage,
                        .srcAccess = Access::None,
                        .dstAccess = access->sync.access,
                        .offsetBytes = access->sync.offsetBytes,
                        .sizeBytes = access->sync.sizeBytes
                    };

                    passBeginBarriers.emplace_back(initialBarrierInfo);
                }
                else
                {
                    const bool hasWriteHazard = lastAccessIt->second.isWrite || access->isWrite;

                    if (hasWriteHazard)
                    {
                        const BufferBarrierInfo barrierInfo {
                            .buffer = *buffer,
                            .srcStage = lastAccessIt->second.sync.stage,
                            .dstStage = access->sync.stage,
                            .srcAccess = lastAccessIt->second.sync.access,
                            .dstAccess = access->sync.access,
                            .offsetBytes = access->sync.offsetBytes,
                            .sizeBytes = access->sync.sizeBytes
                        };

                        passBeginBarriers.emplace_back(barrierInfo);
                    }
                }

                _lastBufferAccessByResource[access->resource] = {
                    .pass = _pass,
                    .sync = access->sync,
                    .isWrite = access->isWrite
                };
            }
        }

        void BuildTextureFinalBarriers(
            const std::unordered_map<FramegraphResource*, FrameGraphBuilder::TextureSynchronization>& _textureFinalStates,
            const std::unordered_map<FramegraphResource*, LastTextureAccessState>& _lastTextureAccessByResource,
            std::unordered_map<IRenderPass*, std::vector<IBarrier>>& _endBarriersByPass
        )
        {
            for (const auto& [resource, finalSync] : _textureFinalStates)
            {
                const auto lastAccessIt = _lastTextureAccessByResource.find(resource);
                CP_EXPECT_MSG(lastAccessIt != _lastTextureAccessByResource.end(), "Texture final state declared without any resource access");

                ITexture* texture = resource->GetTexture();
                CP_EXPECT_MSG(texture != nullptr, "Texture resource is not allocated while building final barriers");

                auto& passEndBarriers = _endBarriersByPass[lastAccessIt->second.pass];

                const TextureBarrierInfo barrierInfo {
                    .texture = *texture,
                    .srcLayout = lastAccessIt->second.sync.layout,
                    .dstLayout = finalSync.layout,
                    .srcStage = lastAccessIt->second.sync.stage,
                    .dstStage = finalSync.stage,
                    .srcAccess = lastAccessIt->second.sync.access,
                    .dstAccess = finalSync.access,
                    .baseMip = finalSync.baseMip,
                    .mipCount = finalSync.mipCount,
                    .baseLayer = finalSync.baseLayer,
                    .layerCount = finalSync.layerCount
                };

                passEndBarriers.emplace_back(barrierInfo);
            }
        }

        void BuildBufferFinalBarriers(
            const std::unordered_map<FramegraphResource*, FrameGraphBuilder::BufferSynchronization>& _bufferFinalStates,
            const std::unordered_map<FramegraphResource*, LastBufferAccessState>& _lastBufferAccessByResource,
            std::unordered_map<IRenderPass*, std::vector<IBarrier>>& _endBarriersByPass
        )
        {
            for (const auto& [resource, finalSync] : _bufferFinalStates)
            {
                const auto lastAccessIt = _lastBufferAccessByResource.find(resource);
                CP_EXPECT_MSG(lastAccessIt != _lastBufferAccessByResource.end(), "Buffer final state declared without any resource access");

                IBuffer* buffer = resource->GetBuffer();
                CP_EXPECT_MSG(buffer != nullptr, "Buffer resource is not allocated while building final barriers");

                auto& passEndBarriers = _endBarriersByPass[lastAccessIt->second.pass];

                const BufferBarrierInfo barrierInfo {
                    .buffer = *buffer,
                    .srcStage = lastAccessIt->second.sync.stage,
                    .dstStage = finalSync.stage,
                    .srcAccess = lastAccessIt->second.sync.access,
                    .dstAccess = finalSync.access,
                    .offsetBytes = finalSync.offsetBytes,
                    .sizeBytes = finalSync.sizeBytes
                };

                passEndBarriers.emplace_back(barrierInfo);
            }
        }
    }

    void BarrierManager::BuildPassBarriers(
        const std::vector<IRenderPass*>& _executionOrder,
        const std::vector<FrameGraphBuilder::TextureResourceAccess>& _textureAccesses,
        const std::vector<FrameGraphBuilder::BufferResourceAccess>& _bufferAccesses,
        const std::unordered_map<FramegraphResource*, FrameGraphBuilder::TextureSynchronization>& _textureFinalStates,
        const std::unordered_map<FramegraphResource*, FrameGraphBuilder::BufferSynchronization>& _bufferFinalStates
    )
    {
        beginBarriersByPass.clear();
        endBarriersByPass.clear();

        const TextureAccessByPass textureAccessesByPass = BuildTextureAccessesByPass(_executionOrder, _textureAccesses);
        const BufferAccessByPass bufferAccessesByPass = BuildBufferAccessesByPass(_executionOrder, _bufferAccesses);

        std::unordered_map<FramegraphResource*, LastTextureAccessState> lastTextureAccessByResource;
        lastTextureAccessByResource.reserve(_textureAccesses.size());

        std::unordered_map<FramegraphResource*, LastBufferAccessState> lastBufferAccessByResource;
        lastBufferAccessByResource.reserve(_bufferAccesses.size());

        for (IRenderPass* pass : _executionOrder)
        {
            BuildPassTextureBeginBarriers(pass, textureAccessesByPass, beginBarriersByPass, lastTextureAccessByResource);
            BuildPassBufferBeginBarriers(pass, bufferAccessesByPass, beginBarriersByPass, lastBufferAccessByResource);
        }

        BuildTextureFinalBarriers(_textureFinalStates, lastTextureAccessByResource, endBarriersByPass);
        BuildBufferFinalBarriers(_bufferFinalStates, lastBufferAccessByResource, endBarriersByPass);
    }

    const std::vector<IBarrier>& BarrierManager::GetPassBeginBarriers(IRenderPass* _pass) const
    {
        if (const auto it = beginBarriersByPass.find(_pass); it != beginBarriersByPass.end())
        {
            return it->second;
        }

        return EmptyBarriers();
    }

    const std::vector<IBarrier>& BarrierManager::GetPassEndBarriers(IRenderPass* _pass) const
    {
        if (const auto it = endBarriersByPass.find(_pass); it != endBarriersByPass.end())
        {
            return it->second;
        }

        return EmptyBarriers();
    }

    void BarrierManager::Clear()
    {
        beginBarriersByPass.clear();
        endBarriersByPass.clear();
    }
}

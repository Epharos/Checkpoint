#pragma once

#include <iostream>

#include "Renderpass.hpp"
#include "FrameGraphBuilder.hpp"

#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <Common/Data/Extent.hpp>

namespace cp
{
    struct NegativePostFXData
    {
        FramegraphResourceHandle* finalRendering = nullptr;

        // External dependencies set by Renderer
        std::shared_ptr<IPipeline> pipeline;
        std::shared_ptr<IDescriptorSet> descriptorSet;
        Extent2D<uint32_t> renderExtent;
    };

    class NegativePostFX : public RenderPass<NegativePostFXData>
    {
    public:
        NegativePostFX()
        {
            name = "Negative Post-FX";
        }

        void Setup(FrameGraphBuilder &_builder) override
        {
            constexpr FrameGraphBuilder::TextureSynchronization synchronization
            {
                .layout = TextureLayout::General,
                .stage = PipelineStage::ComputeShader,
                .access = Access::ShaderRead | Access::ShaderWrite
            };

            data.finalRendering = _builder.UseTexture("FinalRendering", synchronization, ResourceUsage::ReadWrite);
        }

        void Execute(RenderPassExecutionContext &_context) override
        {
            ICommandBuffer& cmd = _context; // For now we use the primary command buffer (graphics but supports everything)

            if (data.pipeline && data.descriptorSet)
            {
                cmd.BindPipeline(*data.pipeline);
                cmd.BindDescriptorSet(0, *data.descriptorSet);
                cmd.Dispatch(data.renderExtent.x(), data.renderExtent.y(), 1);
            }
        }

        void OnPostCompile(FrameGraph &_framegraph) override
        {
            const DescriptorTextureBinding textureBinding {
                .binding = 0,
                .texture = data.finalRendering->GetTexture(),
                .layout = TextureLayout::General,
            };

            data.descriptorSet->UpdateTextures({ textureBinding });
        }
    };
}

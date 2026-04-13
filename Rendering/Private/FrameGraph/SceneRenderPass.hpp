#pragma once

#include "Renderpass.hpp"
#include "FrameGraphBuilder.hpp"

#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <Common/Data/Extent.hpp>
#include <Common/Data/Viewport.hpp>
#include <Common/Data/Rectangle.hpp>

namespace cp
{
    struct SceneRenderPassData
    {
        FramegraphResourceHandle* finalRendering = nullptr;

        // External dependencies set by Renderer
        std::shared_ptr<IPipeline> pipeline;
        std::shared_ptr<IDescriptorSet> descriptorSet;
        Extent2D<uint32_t> renderExtent;
    };

    class SceneRenderPass : public RenderPass<SceneRenderPassData>
    {
    public:
        SceneRenderPass()
        {
            name = "SceneRenderPass";
        }

        void Setup(FrameGraphBuilder& _builder) override
        {
            // Create final rendering texture (will be blitted to swapchain)
            const TextureInfo finalInfo {
                .type = TextureType::Texture2D,
                .extent = Extent3D<uint32_t>{ data.renderExtent.x(), data.renderExtent.y(), 1 },
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = Format::R8G8B8A8_UNORM,
                .usage = TextureUsage::ColorAttachment | TextureUsage::Storage | TextureUsage::TransferSrc,
                .aspect = TextureAspect::Color,
            };

            data.finalRendering = _builder.CreateTexture("FinalRendering", finalInfo);
            _builder.UseTexture(data.finalRendering, {
                .layout = TextureLayout::ColorAttachment,
                .stage = PipelineStage::ColorAttachment,
                .access = Access::ColorAttachmentWrite
            }, ResourceUsage::WriteOnly);

            _builder.SetTextureFinalState(data.finalRendering, {
                .layout = TextureLayout::TransferSrc,
                .stage = PipelineStage::Transfer,
                .access = Access::TransferRead
            });
        }

        void Execute(RenderPassExecutionContext& _context) override
        {
            ICommandBuffer& cmd = _context;
            ITexture* finalTexture = data.finalRendering->GetTexture();

            // Setup rendering
            const ColorAttachmentInfo colorAttachment{
                .texture = finalTexture,
                .loadOp = LoadOp::Clear,
                .storeOp = StoreOp::Store,
                .clearValue = Color(ColorRGBA8(255, 255, 0, 255))  // Yellow clear
            };

            const RenderingInfo renderingInfo{
                .extent = data.renderExtent,
                .layers = 1,
                .colorAttachments = { colorAttachment },
                .depthStencilAttachment = std::nullopt
            };

            cmd.SetViewport(Viewport {
                0.f,
                0.f,
                static_cast<float>(data.renderExtent.x()),
                static_cast<float>(data.renderExtent.y())
            });

            cmd.SetScissor(Rectangle2D {
                Extent2D { 0, 0 },
                data.renderExtent
            });

            cmd.BeginRendering(renderingInfo);

            // Draw the scene
            if (data.pipeline && data.descriptorSet)
            {
                cmd.BindPipeline(*data.pipeline);
                cmd.BindDescriptorSet(0, *data.descriptorSet);
                cmd.Draw(6, 1, 0, 0);  // Fullscreen rectangle
            }

            cmd.EndRendering();

        }
    };
}

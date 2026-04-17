#pragma once

#include <memory>
#include <vector>

#include <Common/Core/Assert.hpp>
#include <Common/IO/FileHelper.hpp>
#include <Common/Data/Viewport.hpp>
#include <Common/Data/Rectangle.hpp>

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>

#include <Rendering/FrameGraph/Renderpass.hpp>
#include <Rendering/FrameGraph/FrameGraphBuilder.hpp>

#include <Resources/AssetRegistry.hpp>
#include <Resources/AssetHandle.hpp>

namespace cp
{
    struct SceneRenderPassData
    {
        FramegraphResourceHandle* finalRendering = nullptr;
        Extent2D<uint32_t> renderExtent {};

        AssetHandle<ITexture> logoTexture;
        std::shared_ptr<ISampler> logoSampler;
        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IDescriptorSet> descriptorSet;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;
    };

    class SceneRenderPass : public RenderPass<SceneRenderPassData>, public IConfigurableRenderPass
    {
    public:
        SceneRenderPass()
        {
            name = "SceneRenderPass";
        }

        void Configure(const RenderPassInitContext& _context) override
        {
            data.renderExtent = _context.renderExtent;

            const auto shaderPath = FindFileInParentTree("Logo.spv");
            CP_ASSERT_MSG(!shaderPath.empty(), "Could not find Logo.spv from current working directory hierarchy");
            const std::vector<uint8_t> shaderBytecode = LoadBinaryFile(shaderPath);

            const ShaderModuleInfo shaderModuleInfo
            {
                .stages = ShaderStage::Vertex | ShaderStage::Fragment,
                .bytecode = ShaderBytecode {
                    .format = ShaderBinaryFormat::SpirV,
                    .data = shaderBytecode.data(),
                    .sizeBytes = shaderBytecode.size()
                }
            };

            data.shaderModule = _context.rhi.GetDevice().CreateShaderModule(shaderModuleInfo);

            const DescriptorSetLayoutInfo descriptorSetLayoutInfo
            {
                .bindings = {
                    DescriptorBinding {
                        .binding = 0,
                        .type = DescriptorType::CombinedImageSampler,
                        .count = 1,
                        .visibility = ShaderStage::Fragment
                    }
                }
            };

            data.descriptorSetLayout = _context.rhi.GetDevice().CreateDescriptorSetLayout(descriptorSetLayoutInfo);
            data.descriptorSet = _context.rhi.GetDevice().CreateDescriptorSet(*data.descriptorSetLayout);

            auto& textureManager = _context.assetRegistry.Get<ITexture>();
            data.logoTexture = textureManager.Load(FindFileInParentTree("logo.png"));
            data.logoSampler = _context.rhi.CreateSampler(SamplerInfo{});

            const DescriptorTextureBinding textureBinding {
                .binding = 0,
                .texture = data.logoTexture.Get(),
                .layout = TextureLayout::ShaderReadOnly,
                .sampler = data.logoSampler.get()
            };

            data.descriptorSet->UpdateTextures({ textureBinding });

            const PipelineLayoutInfo pipelineLayoutInfo {
                .setLayouts = { data.descriptorSetLayout }
            };
            data.pipelineLayout = _context.rhi.GetDevice().CreatePipelineLayout(pipelineLayoutInfo);

            const GraphicsPipelineInfo graphicsPipelineInfo
            {
                .layout = data.pipelineLayout,
                .shaderModule = data.shaderModule,
                .stageMains = {
                    { ShaderStage::Vertex, "vertexMain" },
                    { ShaderStage::Fragment, "fragmentMain" },
                },
                .vertexInput = VertexInputState{},
                .topology = PrimitiveTopology::TriangleList,
                .rasterization = RasterizationState{},
                .depthStencil = DepthStencilState{},
                .blendAttachments = {},
                .colorAttachmentFormats = { Format::R8G8B8A8_UNORM },
                .depthStencilFormat = Format::D24_UNORM_S8_UINT
            };

            data.pipeline = _context.rhi.GetDevice().CreateGraphicsPipeline(graphicsPipelineInfo);
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

        void OnReset() override
        {
            data.finalRendering = nullptr;
        }
    };
}

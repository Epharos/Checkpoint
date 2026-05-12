#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Common/Data/Rectangle.hpp>
#include <Common/Data/Viewport.hpp>
#include <Common/IO/FileHelper.hpp>

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Rendering/Camera.hpp>
#include <Rendering/DebugDraw.hpp>
#include <Rendering/FrameGraph/FrameGraph.hpp>
#include <Rendering/FrameGraph/FrameGraphBuilder.hpp>
#include <Rendering/FrameGraph/Renderpass.hpp>

#include <glm/glm.hpp>

namespace cp
{
    inline constexpr std::string_view DebugDrawPassTypeName = "DebugDrawPass";

    struct DebugDrawPassData
    {
        struct CameraUbo
        {
            glm::mat4 viewProjection{ 1.0f };
        };

        struct LineVertex
        {
            float x, y, z;
            float r, g, b, a;
        };

        FramegraphResourceHandle* finalRendering = nullptr;

        Extent2D<uint32_t> renderExtent {};
        RenderingHardwareInterface* rhi = nullptr;

        std::shared_ptr<DebugDrawBuffer> debugDrawBuffer;

        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;
        std::shared_ptr<IBuffer> cameraUboBuffer;

        std::array<std::shared_ptr<IBuffer>, 3> vertexBuffers {};
        size_t transientFrameIndex = 0;
    };

    class DebugDrawPass : public RenderPass<DebugDrawPassData>, public IConfigurableRenderPass
    {
    public:
        DebugDrawPass()
        {
            name = "DebugDrawPass";
        }

        void Configure(const RenderPassInitContext& _context) override
        {
            data.renderExtent = _context.renderExtent;
            data.rhi = &_context.rhi;

            const auto shaderPath = FindFileInParentTree("DebugLine.spv");
            if (shaderPath.empty())
            {
                return;
            }

            const std::vector<uint8_t> shaderBytecode = LoadBinaryFile(shaderPath);
            if (shaderBytecode.empty())
            {
                return;
            }

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
            if (!data.shaderModule)
            {
                return;
            }

            const DescriptorSetLayoutInfo descriptorSetLayoutInfo
            {
                .bindings = {
                    DescriptorBinding {
                        .binding = 0,
                        .type = DescriptorType::UniformBuffer,
                        .count = 1,
                        .visibility = ShaderStage::Vertex
                    }
                }
            };

            data.descriptorSetLayout = _context.rhi.GetDevice().CreateDescriptorSetLayout(descriptorSetLayoutInfo);

            const PipelineLayoutInfo pipelineLayoutInfo {
                .setLayouts = { data.descriptorSetLayout }
            };

            data.pipelineLayout = _context.rhi.GetDevice().CreatePipelineLayout(pipelineLayoutInfo);

            const GraphicsPipelineInfo graphicsPipelineInfo
            {
                .layout = data.pipelineLayout,
                .shaderModule = data.shaderModule,
                .stageMains = {
                    { ShaderStage::Vertex,   "VSMain" },
                    { ShaderStage::Fragment, "FSMain" },
                },
                .vertexInput = VertexInputState{
                    .bindings = {
                        VertexBindingInfo {
                            .binding = 0,
                            .strideBytes = static_cast<uint32_t>(sizeof(DebugDrawPassData::LineVertex)),
                            .inputRate = VertexInputRate::PerVertex
                        }
                    },
                    .attributes = {
                        VertexAttributeInfo {
                            .location = 0,
                            .binding = 0,
                            .format = VertexFormat::R32G32B32_FLOAT,
                            .offsetBytes = static_cast<uint32_t>(offsetof(DebugDrawPassData::LineVertex, x))
                        },
                        VertexAttributeInfo {
                            .location = 1,
                            .binding = 0,
                            .format = VertexFormat::R32G32B32A32_FLOAT,
                            .offsetBytes = static_cast<uint32_t>(offsetof(DebugDrawPassData::LineVertex, r))
                        }
                    }
                },
                .topology = PrimitiveTopology::LineList,
                .rasterization = RasterizationState {
                    .cullMode = CullMode::None
                },
                .depthStencil = {},
                .blendAttachments = {},
                .colorAttachmentFormats = { Format::R8G8B8A8_UNORM },
            };

            data.pipeline = _context.rhi.GetDevice().CreateGraphicsPipeline(graphicsPipelineInfo);

            const BufferInfo uboInfo {
                .sizeBytes = sizeof(DebugDrawPassData::CameraUbo),
                .usage = BufferUsage::Uniform,
                .cpuVisible = true
            };

            data.cameraUboBuffer = _context.rhi.CreateBuffer(uboInfo);
        }

        void DeclareDependencies(FrameGraph& _framegraph) override
        {
            _framegraph.AddPassDependencyIfPresent(GetName(), "SkyboxPass");
            _framegraph.AddPassDependencyIfPresent(GetName(), "OpaqueMaterialPass");
        }

        void Setup(FrameGraphBuilder& _builder) override
        {
            data.finalRendering = _builder.UseTexture("FinalRendering", {
                .layout = TextureLayout::ColorAttachment,
                .stage  = PipelineStage::ColorAttachment,
                .access = Access::ColorAttachmentWrite
            }, ResourceUsage::ReadWrite);

            data.debugDrawBuffer = _builder.GetCpuResource<DebugDrawBuffer>("DebugDraw.Buffer");
        }

        void Execute(RenderPassExecutionContext& _context) override
        {
            if (!data.pipeline || !data.cameraUboBuffer)
                return;
            if (!data.debugDrawBuffer || data.debugDrawBuffer->IsEmpty())
                return;
            if (!_context.camera.isValid)
                return;

            ICommandBuffer& cmd = _context;
            ITexture* colorTarget = data.finalRendering ? data.finalRendering->GetTexture() : nullptr;
            if (!colorTarget)
                return;

            // Upload camera UBO
            {
                const DebugDrawPassData::CameraUbo uboData { .viewProjection = _context.camera.viewProjection };
                void* mapped = data.cameraUboBuffer->Map();
                if (!mapped) return;
                std::memcpy(mapped, &uboData, sizeof(uboData));
                data.cameraUboBuffer->Unmap();
            }

            // Build vertex buffer for this frame
            const std::vector<DebugLine>& lines = data.debugDrawBuffer->GetLines();
            const size_t vertexCount = lines.size() * 2;

            data.transientFrameIndex = (data.transientFrameIndex + 1) % data.vertexBuffers.size();
            std::shared_ptr<IBuffer>& vertexBuffer = data.vertexBuffers[data.transientFrameIndex];

            const size_t neededBytes = vertexCount * sizeof(DebugDrawPassData::LineVertex);
            const bool needsRealloc = !vertexBuffer || vertexBuffer->GetSizeBytes() < neededBytes;
            if (needsRealloc)
            {
                const BufferInfo vbInfo {
                    .sizeBytes = neededBytes,
                    .usage = BufferUsage::Vertex,
                    .cpuVisible = true
                };
                vertexBuffer = data.rhi->CreateBuffer(vbInfo);
                if (!vertexBuffer) return;
            }

            {
                void* mapped = vertexBuffer->Map();
                if (!mapped) return;
                auto* dst = static_cast<DebugDrawPassData::LineVertex*>(mapped);
                for (const DebugLine& line : lines)
                {
                    *dst++ = { line.start.x, line.start.y, line.start.z,
                               line.color.r, line.color.g, line.color.b, line.color.a };
                    *dst++ = { line.end.x,   line.end.y,   line.end.z,
                               line.color.r, line.color.g, line.color.b, line.color.a };
                }
                vertexBuffer->Unmap();
            }

            const std::shared_ptr<IDescriptorSet> descriptorSet =
                data.rhi->GetDevice().CreateDescriptorSet(*data.descriptorSetLayout);
            if (!descriptorSet) return;

            descriptorSet->UpdateBuffers({
                DescriptorBufferBinding {
                    .binding = 0,
                    .buffer = data.cameraUboBuffer.get(),
                    .offsetBytes = 0,
                    .rangeBytes = sizeof(DebugDrawPassData::CameraUbo)
                }
            });

            // Record draw
            const ColorAttachmentInfo colorAttachment {
                .texture = colorTarget,
                .loadOp = LoadOp::Load,
                .storeOp = StoreOp::Store,
                .clearValue = {}
            };

            const RenderingInfo renderingInfo {
                .extent = data.renderExtent,
                .layers = 1,
                .colorAttachments = { colorAttachment },
            };

            cmd.SetViewport(Viewport {
                0.f, 0.f,
                static_cast<float>(data.renderExtent.x()),
                static_cast<float>(data.renderExtent.y())
            });
            cmd.SetScissor(Rectangle2D {
                Extent2D{ 0, 0 },
                data.renderExtent
            });

            cmd.BeginRendering(renderingInfo);
            cmd.BindPipeline(*data.pipeline);
            cmd.BindDescriptorSet(0, *descriptorSet);
            cmd.BindVertexBuffer(0, *vertexBuffer);
            cmd.Draw(static_cast<uint32_t>(vertexCount), 1, 0, 0);
            cmd.EndRendering();
        }

        void OnReset() override
        {
            data.finalRendering = nullptr;
            data.debugDrawBuffer.reset();
            for (auto& buf : data.vertexBuffers)
                buf.reset();
        }

    private:
        void OnPreCompile(FrameGraph& _framegraph) override {}
        void OnPostCompile(FrameGraph& _framegraph) override {}
    };
}

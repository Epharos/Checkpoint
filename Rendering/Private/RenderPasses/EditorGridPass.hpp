#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

#include <Rendering/FrameGraph/FrameGraph.hpp>
#include <Rendering/FrameGraph/FrameGraphBuilder.hpp>
#include <Rendering/FrameGraph/Renderpass.hpp>
#include <Rendering/IShaderProvider.hpp>

#include <glm/glm.hpp>

namespace cp
{
    inline constexpr std::string_view EditorGridPassTypeName = "EditorGridPass";

    struct EditorGridPassData
    {
        struct alignas(16) CameraUbo
        {
            glm::mat4 viewProjection { 1.0f };
            glm::mat4 inverseViewProjection { 1.0f };
            glm::vec4 worldPositionAndNear { 0.0f, 0.0f, 0.0f, 0.1f };  // xyz=pos, w=near
            glm::vec4 farAndPadding { 1000.0f, 0.0f, 0.0f, 0.0f }; // x=far
        };

        struct alignas(16) GridParamsUbo
        {
            glm::vec4  color { 0.5f, 0.5f, 0.5f, 0.8f };
            float cellSize { 1.0f };
            float lineThickness { 0.02f };
            float fadeStartDistance { 50.0f };
            float fadeEndDistance { 100.0f };
            uint32_t ignoreDepth { 0 };
            float padding[3] = { 0.f, 0.f, 0.f };
        };

        FramegraphResourceHandle* finalRendering = nullptr;
        FramegraphResourceHandle* depthBuffer = nullptr;
        Extent2D<uint32_t> renderExtent {};

        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IDescriptorSet> descriptorSet;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;
        std::shared_ptr<IBuffer> cameraUboBuffer;
        std::shared_ptr<IBuffer> gridParamsUboBuffer;
    };

    class EditorGridPass : public RenderPass<EditorGridPassData>, public IConfigurableRenderPass
    {
    public:
        EditorGridPass() { name = "EditorGridPass"; }

        [[nodiscard]] bool GetEnabled() const { return enabled; }
        void SetEnabled(bool _v) { enabled = _v; }

        [[nodiscard]] glm::vec3 GetColor() const { return { params.color.r, params.color.g, params.color.b }; }
        void SetColor(const glm::vec3& _rgb)
        {
            params.color.r = _rgb.r;
            params.color.g = _rgb.g;
            params.color.b = _rgb.b;
            UploadParams();
        }

        [[nodiscard]] float GetAlpha() const { return params.color.a; }
        void SetAlpha(float _a)
        {
            params.color.a = _a;
            UploadParams();
        }

        [[nodiscard]] float GetCellSize() const { return params.cellSize; }
        void SetCellSize(float _s)
        {
            params.cellSize = glm::max(0.0001f, _s);
            UploadParams();
        }

        [[nodiscard]] float GetLineThickness() const { return params.lineThickness; }
        void SetLineThickness(float _t)
        {
            params.lineThickness = glm::clamp(_t, 0.001f, 0.499f);
            UploadParams();
        }

        [[nodiscard]] float GetFadeStartDistance() const { return params.fadeStartDistance; }
        void SetFadeStartDistance(float _d)
        {
            params.fadeStartDistance = _d;
            UploadParams();
        }

        [[nodiscard]] float GetFadeEndDistance() const { return params.fadeEndDistance; }
        void SetFadeEndDistance(float _d)
        {
            params.fadeEndDistance = _d;
            UploadParams();
        }

        [[nodiscard]] bool GetIgnoreDepth() const { return params.ignoreDepth != 0; }
        void SetIgnoreDepth(bool _v)
        {
            params.ignoreDepth = _v ? 1u : 0u;
            UploadParams();
        }

        void Serialize(cp::ISerializer& _s) const override
        {
            _s.WritePod(enabled);
            _s.WritePod(params.color);
            _s.WritePod(params.cellSize);
            _s.WritePod(params.lineThickness);
            _s.WritePod(params.fadeStartDistance);
            _s.WritePod(params.fadeEndDistance);
            _s.WritePod(params.ignoreDepth);
        }

        void Deserialize(cp::IDeserializer& _d) override
        {
            _d.ReadPod(enabled);
            _d.ReadPod(params.color);
            _d.ReadPod(params.cellSize);
            _d.ReadPod(params.lineThickness);
            _d.ReadPod(params.fadeStartDistance);
            _d.ReadPod(params.fadeEndDistance);
            _d.ReadPod(params.ignoreDepth);
        }

        void Configure(const RenderPassInitContext& _ctx) override
        {
            data.renderExtent = _ctx.renderExtent;

            if (!_ctx.shaderProvider)
            {
                return;
            }

            const ShaderProviderResult shader = _ctx.shaderProvider->GetShader("EditorGrid.slang");
            if (!shader.success || shader.binary.empty())
            {
                return;
            }

            data.shaderModule = _ctx.rhi.GetDevice().CreateShaderModule({
                .stages = ShaderStage::Compute,
                .bytecode = {
                    .format = shader.format,
                    .data = shader.binary.data(),
                    .sizeBytes = shader.binary.size()
                }
            });

            if (!data.shaderModule)
            {
                return;
            }

            data.descriptorSetLayout = _ctx.rhi.GetDevice().CreateDescriptorSetLayout({
                .bindings = {
                    {
                        .binding = 0,
                        .type = DescriptorType::StorageTexture,
                        .count = 1,
                        .visibility = ShaderStage::Compute
                    },
                    {
                        .binding = 1,
                        .type = DescriptorType::SampledTexture,
                        .count = 1,
                        .visibility = ShaderStage::Compute
                    },
                    {
                        .binding = 2,
                        .type = DescriptorType::UniformBuffer,
                        .count = 1,
                        .visibility = ShaderStage::Compute
                    },
                    {
                        .binding = 3,
                        .type = DescriptorType::UniformBuffer,
                        .count = 1,
                        .visibility = ShaderStage::Compute
                    },
                }
            });

            data.descriptorSet = _ctx.rhi.GetDevice().CreateDescriptorSet(*data.descriptorSetLayout);
            data.pipelineLayout = _ctx.rhi.GetDevice().CreatePipelineLayout({ .setLayouts = { data.descriptorSetLayout } });
            data.pipeline = _ctx.rhi.GetDevice().CreateComputePipeline({
                .layout = data.pipelineLayout,
                .computeShader = data.shaderModule,
                .mainMethodName = "MainCS"
            });

            data.cameraUboBuffer = _ctx.rhi.CreateBuffer({
                .sizeBytes = sizeof(EditorGridPassData::CameraUbo),
                .usage = BufferUsage::Uniform,
                .cpuVisible = true
            });
            data.gridParamsUboBuffer = _ctx.rhi.CreateBuffer({
                .sizeBytes = sizeof(EditorGridPassData::GridParamsUbo),
                .usage = BufferUsage::Uniform,
                .cpuVisible = true
            });

            UploadParams();
        }

        void DeclareDependencies(FrameGraph& _fg) override
        {
            _fg.AddPassDependencyIfPresent(GetName(), "DebugDrawPass");
        }

        void Setup(FrameGraphBuilder& _builder) override
        {
            data.finalRendering = _builder.UseTexture("FinalRendering", {
                .layout = TextureLayout::General,
                .stage = PipelineStage::ComputeShader,
                .access = Access::ShaderRead | Access::ShaderWrite
            }, ResourceUsage::ReadWrite);

            data.depthBuffer = _builder.UseTexture("DepthBuffer", {
                .layout = TextureLayout::ShaderReadOnly,
                .stage = PipelineStage::ComputeShader,
                .access = Access::ShaderRead
            }, ResourceUsage::ReadOnly);
        }

        void Execute(RenderPassExecutionContext& _ctx) override
        {
            if (!enabled || _ctx.environment != RendererEnvironment::Editor)
                return;
            if (!_ctx.camera.isValid)
                return;
            if (!data.pipeline || !data.descriptorSet || !data.cameraUboBuffer)
                return;

            const EditorGridPassData::CameraUbo camUbo {
                .viewProjection = _ctx.camera.viewProjection,
                .inverseViewProjection = glm::inverse(_ctx.camera.viewProjection),
                .worldPositionAndNear  = glm::vec4(_ctx.camera.worldPosition, _ctx.camera.nearPlane),
                .farAndPadding = glm::vec4(_ctx.camera.farPlane, 0.0f, 0.0f, 0.0f),
            };

            if (void* mapped = data.cameraUboBuffer->Map())
            {
                std::memcpy(mapped, &camUbo, sizeof(camUbo));
                data.cameraUboBuffer->Unmap();
            }

            ICommandBuffer& cmd = _ctx;
            cmd.BindPipeline(*data.pipeline);
            cmd.BindDescriptorSet(0, *data.descriptorSet);
            cmd.Dispatch(data.renderExtent.x(), data.renderExtent.y(), 1);
        }

        void OnPostCompile(FrameGraph&) override
        {
            if (!data.descriptorSet || !data.finalRendering || !data.depthBuffer)
                return;
            if (!data.cameraUboBuffer || !data.gridParamsUboBuffer)
                return;

            data.descriptorSet->UpdateTextures({
                {
                    .binding = 0,
                    .texture = data.finalRendering->GetTexture(),
                    .layout = TextureLayout::General
                },
                {
                    .binding = 1,
                    .texture = data.depthBuffer->GetTexture(),
                    .layout = TextureLayout::ShaderReadOnly
                },
            });
            data.descriptorSet->UpdateBuffers({
                {
                    .binding = 2,
                    .buffer = data.cameraUboBuffer.get(),
                    .offsetBytes = 0,
                    .rangeBytes = sizeof(EditorGridPassData::CameraUbo)
                },
                {
                    .binding = 3,
                    .buffer = data.gridParamsUboBuffer.get(),
                    .offsetBytes = 0,
                    .rangeBytes = sizeof(EditorGridPassData::GridParamsUbo)
                },
            });
        }

        void OnReset() override
        {
            data.finalRendering = nullptr;
            data.depthBuffer = nullptr;
        }

    private:
        bool enabled = true;
        EditorGridPassData::GridParamsUbo params {};

        void UploadParams()
        {
            if (!data.gridParamsUboBuffer)
                return;

            if (void* mapped = data.gridParamsUboBuffer->Map())
            {
                std::memcpy(mapped, &params, sizeof(params));
                data.gridParamsUboBuffer->Unmap();
            }
        }
    };
}

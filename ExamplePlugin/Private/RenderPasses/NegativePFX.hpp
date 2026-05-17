#pragma once

#include <memory>
#include <vector>

#include <Common/IO/FileHelper.hpp>

#include "../ExamplePluginDir.hpp"

#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>

#include <Rendering/FrameGraph/FrameGraph.hpp>
#include <Rendering/IShaderProvider.hpp>

namespace cp
{
    inline constexpr std::string_view NegativePostFXTypeName = "NegativePostFX";

    struct NegativePostFXData
    {
        FramegraphResourceHandle* finalRendering = nullptr;
        Extent2D<uint32_t> renderExtent {};

        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IDescriptorSet> descriptorSet;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;
    };

    class NegativePostFX : public RenderPass<NegativePostFXData>, public IConfigurableRenderPass
    {
    public:
        NegativePostFX()
        {
            name = "Negative Post-FX";
        }

        [[nodiscard]] bool GetEnabled() const { return enabled; }
        void SetEnabled(bool _enabled) { enabled = _enabled; }

        void Serialize(cp::ISerializer& _serializer) const override
        {
            _serializer.WritePod(enabled);
        }

        void Deserialize(cp::IDeserializer& _deserializer) override
        {
            [[maybe_unused]] bool ok = _deserializer.ReadPod(enabled);
        }

        void Configure(const RenderPassInitContext& _context) override
        {
            data.renderExtent = _context.renderExtent;

            if (!_context.shaderProvider)
            {
                return;
            }

            const ShaderProviderResult shader = _context.shaderProvider->GetShader(FindFileFromDirectory("Shaders/NegativePFX.slang", GetExamplePluginDir()));

            if (!shader.success || shader.binary.empty())
            {
                return;
            }

            const ShaderModuleInfo shaderModuleInfo
            {
                .stages = ShaderStage::Compute,
                .bytecode = ShaderBytecode {
                    .format = shader.format,
                    .data = shader.binary.data(),
                    .sizeBytes = shader.binary.size()
                }
            };
            data.shaderModule = _context.rhi.GetDevice().CreateShaderModule(shaderModuleInfo);

            const DescriptorSetLayoutInfo descriptorSetLayoutInfo
            {
                .bindings = {
                    DescriptorBinding {
                        .binding = 0,
                        .type = DescriptorType::StorageTexture,
                        .count = 1,
                        .visibility = ShaderStage::Compute
                    }
                }
            };
            data.descriptorSetLayout = _context.rhi.GetDevice().CreateDescriptorSetLayout(descriptorSetLayoutInfo);
            data.descriptorSet = _context.rhi.GetDevice().CreateDescriptorSet(*data.descriptorSetLayout);

            const PipelineLayoutInfo pipelineLayoutInfo {
                .setLayouts = { data.descriptorSetLayout }
            };
            data.pipelineLayout = _context.rhi.GetDevice().CreatePipelineLayout(pipelineLayoutInfo);

            const ComputePipelineInfo computePipelineInfo
            {
                .layout = data.pipelineLayout,
                .computeShader = data.shaderModule,
                .mainMethodName = "MainCS"
            };
            data.pipeline = _context.rhi.GetDevice().CreateComputePipeline(computePipelineInfo);
        }

        void DeclareDependencies(FrameGraph& _framegraph) override
        {
            _framegraph.AddPassDependencyIfPresent(GetName(), "OpaqueMaterialPass");
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
            if (!enabled)
                return;

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

        void OnReset() override
        {
            data.finalRendering = nullptr;
        }

    private:
        bool enabled = true;
    };
}

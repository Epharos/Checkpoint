#include "Material.hpp"

#include <Common/IO/FileHelper.hpp>
#include <Rendering/IShaderProvider.hpp>
#include <RHI/RenderingHardwareInterface.hpp>

namespace cp
{
    static bool BindingKindToDescriptorType(BindingKind _kind, DescriptorType& _out)
    {
        switch (_kind)
        {
        case BindingKind::ConstantBuffer:
            _out = DescriptorType::UniformBuffer;
            return true;

        case BindingKind::StructuredBuffer:
        case BindingKind::RWStructuredBuffer:
        case BindingKind::ByteAddressBuffer:
        case BindingKind::RWByteAddressBuffer:
            _out = DescriptorType::StorageBuffer;
            return true;

        case BindingKind::Texture1D:
        case BindingKind::Texture1DArray:
        case BindingKind::Texture2D:
        case BindingKind::Texture2DArray:
        case BindingKind::Texture2DMS:
        case BindingKind::Texture2DMSArray:
        case BindingKind::Texture3D:
        case BindingKind::TextureCube:
        case BindingKind::TextureCubeArray:
            _out = DescriptorType::SampledTexture;
            return true;

        case BindingKind::RWTexture1D:
        case BindingKind::RWTexture1DArray:
        case BindingKind::RWTexture2D:
        case BindingKind::RWTexture2DArray:
        case BindingKind::RWTexture3D:
            _out = DescriptorType::StorageTexture;
            return true;

        case BindingKind::Sampler:
        case BindingKind::SamplerComparison:
            _out = DescriptorType::Sampler;
            return true;

        default:
            return false;
        }
    }

    bool Material::Compile(const MaterialCompileContext& _context)
    {
        if (IsCompiled())
        {
            return true;
        }

        if (shaderPath.empty())
        {
            return false;
        }

        const std::filesystem::path resolved = shaderPath.is_absolute()
            ? shaderPath
            : FindFileFromDirectory(shaderPath.string(), _context.searchRoot);

        if (resolved.empty() || !std::filesystem::exists(resolved))
        {
            return false;
        }

        const ShaderProviderResult shaderResult =
            _context.shaderProvider.GetShader(resolved.stem(), resolved.parent_path());

        if (!shaderResult.success || shaderResult.binary.empty())
        {
            return false;
        }

        if (shaderResult.reflection.bindings.empty())
        {
            return false;
        }

        std::vector<DescriptorBinding> layoutBindings;

        for (const BindingReflection& b : shaderResult.reflection.bindings)
        {
            DescriptorType type{};

            if (!BindingKindToDescriptorType(b.kind, type))
            {
                continue;
            }

            layoutBindings.push_back(DescriptorBinding {
                .binding = b.binding,
                .type = type,
                .count = 1,
                .visibility = ShaderStage::All
            });
        }

        if (layoutBindings.empty())
        {
            return false;
        }

        const ShaderModuleInfo shaderModuleInfo {
            .stages = ShaderStage::Vertex | ShaderStage::Fragment,
            .bytecode = ShaderBytecode{
                .format = shaderResult.format,
                .data = shaderResult.binary.data(),
                .sizeBytes = shaderResult.binary.size()
            }
        };

        auto newShaderModule = _context.rhi.GetDevice().CreateShaderModule(shaderModuleInfo);

        if (!newShaderModule)
        {
            return false;
        }

        auto newDescriptorSetLayout = _context.rhi.GetDevice().CreateDescriptorSetLayout(
            DescriptorSetLayoutInfo{ .bindings = layoutBindings });

        if (!newDescriptorSetLayout)
        {
            return false;
        }

        const PipelineLayoutInfo pipelineLayoutInfo{ .setLayouts = { newDescriptorSetLayout } };
        auto newPipelineLayout = _context.rhi.GetDevice().CreatePipelineLayout(pipelineLayoutInfo);

        if (!newPipelineLayout)
        {
            return false;
        }

        const GraphicsPipelineInfo graphicsPipelineInfo {
            .layout = newPipelineLayout,
            .shaderModule = newShaderModule,
            .stageMains = {
                { ShaderStage::Vertex, "VSMain" },
                { ShaderStage::Fragment, "FSMain" },
            },
            .vertexInput = _context.vertexInput,
            .topology = PrimitiveTopology::TriangleList,
            .rasterization = RasterizationState{},
            .depthStencil = DepthStencilState{},
            .blendAttachments = {},
            .colorAttachmentFormats = _context.colorAttachmentFormats,
            .depthStencilFormat = _context.depthStencilFormat
        };

        auto newPipeline = _context.rhi.GetDevice().CreateGraphicsPipeline(graphicsPipelineInfo);

        if (!newPipeline)
        {
            return false;
        }

        shaderModule = std::move(newShaderModule);
        descriptorSetLayout = std::move(newDescriptorSetLayout);
        pipelineLayout = std::move(newPipelineLayout);
        pipeline = std::move(newPipeline);
        reflection = shaderResult.reflection;

        return true;
    }

    void Material::InvalidateGpuResources()
    {
        pipeline.reset();
        pipelineLayout.reset();
        descriptorSetLayout.reset();
        shaderModule.reset();
        reflection = {};
    }
}

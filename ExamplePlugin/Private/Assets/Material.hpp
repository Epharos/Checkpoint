#pragma once

#include <Resources/AssetLoader.hpp>
#include <Common/ShaderReflection.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>
#include <RHI/Rendering.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace cp
{
    class IShaderProvider; // forward-declared to avoid pulling in Rendering headers everywhere

    /**
     * @brief Context passed to Material::Compile().
     */
    struct MaterialCompileContext
    {
        RenderingHardwareInterface& rhi;
        IShaderProvider& shaderProvider;

        const std::filesystem::path& searchRoot;

        VertexInputState vertexInput;

        std::vector<Format> colorAttachmentFormats;
        Format depthStencilFormat = Format::D32_FLOAT;
    };

    struct Material
    {
        std::filesystem::path shaderPath;

        std::shared_ptr<IShaderModule> shaderModule;
        std::shared_ptr<IDescriptorSetLayout> descriptorSetLayout;
        std::shared_ptr<IPipelineLayout> pipelineLayout;
        std::shared_ptr<IPipeline> pipeline;

        ShaderReflection reflection;

        [[nodiscard]] bool IsValid() const { return !shaderPath.empty(); }
        [[nodiscard]] bool IsCompiled() const { return pipeline != nullptr; }

        /**
         * @brief Build (or rebuild) the GPU pipeline for this shader.
         *
         * If already compiled, returns true.
         * On failure the existing GPU resources are left unchanged and false is returned.
         */
        bool Compile(const MaterialCompileContext& _context);

        /**
         * @brief Release all GPU resources.
         *
         * The next Compile() call will rebuild them.
         */
        void InvalidateGpuResources();
    };

    template<>
    struct AssetLoader<Material>
    {
        static std::shared_ptr<Material> Load(const std::filesystem::path& _path, RenderingHardwareInterface& _rhi);
    };
}

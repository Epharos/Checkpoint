#pragma once

#include "Material.hpp"
#include <Resources/AssetHandle.hpp>
#include <Resources/AssetLoader.hpp>
#include <RHI/Core.hpp>
#include <RHI/Data.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace cp
{
    enum class ParameterValueType : uint8_t
    {
        Bool = 0,
        Int32 = 1,
        Float = 2,
        Float2 = 3,
        Float3 = 4,
        Float4 = 5,
        Texture2D = 6,
    };

    using ParameterValue = std::variant<
        bool,
        int32_t,
        float,
        std::array<float, 2>,
        std::array<float, 3>,
        std::array<float, 4>,
        AssetHandle<ITexture>
    >;

    /**
     * @brief A CPU-visible GPU buffer that holds the packed data for one
     * constant-buffer or storage-buffer binding owned by a MaterialInstance.
     *
     * Pre-allocated when RebuildGpuBuffers() is called; reused across frames
     * so per-frame allocations are avoided.
     */
    struct GpuParamBuffer
    {
        uint32_t binding = 0;
        uint32_t sizeBytes = 0;
        std::shared_ptr<IBuffer> buffer;
    };

    struct MaterialInstance
    {
        AssetHandle<Material> material;
        std::unordered_map<std::string, ParameterValue> parameters;

        std::vector<GpuParamBuffer> gpuParamBuffers;

        bool gpuDirty = true;

        [[nodiscard]] bool IsValid() const { return material.IsValid(); }

        /**
         * @brief Rebuild the CPU-visible GPU buffers from current parameter values.
         *
         * @param _rhi RHI used to allocate the buffers.
         * @param _reflection Shader reflection obtained from Material::reflection.
         */
        void RebuildGpuBuffers(RenderingHardwareInterface& _rhi, const ShaderReflection& _reflection);

        /**
         * @brief Mark GPU buffers as stale.
         *
         * They will be rebuilt by the next RebuildGpuBuffers() call.
         * Call this whenever parameters are modified (e.g. from the authoring UI).
         */
        void InvalidateGpuBuffers() { gpuDirty = true; }
    };

    template<>
    struct AssetLoader<MaterialInstance>
    {
        static std::shared_ptr<MaterialInstance> Load(
            const std::filesystem::path& _path,
            RenderingHardwareInterface& _rhi
        );
    };
}

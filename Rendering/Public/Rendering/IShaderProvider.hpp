#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <RHI/Rendering.hpp>

namespace cp
{
    /** @brief Result returned by IShaderProvider::GetShader(). */
    struct ShaderProviderResult
    {
        bool success = false;

        /** @brief Compiler diagnostics; non-empty when success is false. */
        std::string diagnostics;

        /** @brief Raw SPIR-V or DXIL bytecode; empty when success is false. */
        std::vector<uint8_t> binary;

        ShaderBinaryFormat format = ShaderBinaryFormat::SpirV;
    };

    /**
     * @brief Abstract interface for supplying compiled shader binaries to render passes.
     */
    class IShaderProvider
    {
    public:
        virtual ~IShaderProvider() = default;

        /**
         * @brief Return a compiled shader binary containing all entry points.
         *
         * @param _shaderName Logical shader name without its source extension
         *                    (e.g. "Skybox", "DebugLine").
         *                    The provider resolves the correct file (source or pre-compiled
         *                    binary) and derives the binary format automatically.
         * @param _directory  Optional directory, searched before the provider's own
         *                    default search paths. Use this when the shader lives in a
         *                    plugin-specific folder not known at provider construction time.
         * @return ShaderProviderResult with success == true and binary populated on success.
         */
        [[nodiscard]] virtual ShaderProviderResult GetShader(
            const std::filesystem::path& _shaderName,
            const std::filesystem::path& _directory = {}
        ) = 0;
    };
}

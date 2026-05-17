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
         * @param _shaderName Logical shader name or relative path (e.g. "EditorGrid.slang").
         *                    The provider is responsible for locating the source and deriving
         *                    the binary format.
         * @return ShaderProviderResult with success == true and binary populated on success.
         */
        [[nodiscard]] virtual ShaderProviderResult GetShader(const std::filesystem::path& _shaderName) = 0;
    };
}

#pragma once

#include <Rendering/IShaderProvider.hpp>
#include <Common/IO/FileHelper.hpp>

#include <filesystem>
#include <string>

namespace cp
{
    /**
     * @brief IShaderProvider implementation for standalone runtimes.
     *
     * Resolves a logical shader name (e.g. "EditorGrid.slang") to a pre-compiled
     * binary (.spv or .dxil) by:
     *   1. Looking in the configured @c shaderDir first.
     *   2. Falling back to FindFileInParentTree() for development convenience.
     */
    class PrecompiledShaderProvider final : public cp::IShaderProvider
    {
    public:
        /**
         * @param _shaderDir Directory containing pre-compiled binaries.
         *                   Pass an empty path to rely solely on FindFileInParentTree.
         * @param _format Binary format of the pre-compiled files.
         */
        explicit PrecompiledShaderProvider(
            std::filesystem::path _shaderDir = {},
            cp::ShaderBinaryFormat _format = cp::ShaderBinaryFormat::SpirV
        ) : shaderDir(std::move(_shaderDir)), format(_format) {}

        [[nodiscard]] cp::ShaderProviderResult GetShader(
            const std::filesystem::path& _shaderName,
            const std::filesystem::path& _directory = {}
        ) override
        {
            const std::string ext = (format == cp::ShaderBinaryFormat::SpirV) ? ".spv" : ".dxil";
            const std::string binaryName = _shaderName.stem().string() + ext;

            std::filesystem::path resolved;

            // Caller-supplied directory
            if (!_directory.empty())
            {
                const std::filesystem::path candidate = _directory / binaryName;
                if (std::filesystem::exists(candidate))
                {
                    resolved = candidate;
                }
            }

            // Configured shader directory
            if (resolved.empty() && !shaderDir.empty())
            {
                const std::filesystem::path candidate = shaderDir / binaryName;
                if (std::filesystem::exists(candidate))
                {
                    resolved = candidate;
                }
            }

            if (resolved.empty())
            {
                resolved = cp::FindFileInParentTree(binaryName);
            }

            if (resolved.empty())
            {
                return {
                    .success = false,
                    .diagnostics = "Pre-compiled shader not found: " + binaryName
                };
            }

            std::vector<uint8_t> bytes = cp::LoadBinaryFile(resolved);
            if (bytes.empty())
            {
                return {
                    .success = false,
                    .diagnostics = "Failed to read shader binary: " + resolved.string()
                };
            }

            return cp::ShaderProviderResult {
                .success = true,
                .binary = std::move(bytes),
                .format = format,
            };
        }

    private:
        std::filesystem::path shaderDir;
        cp::ShaderBinaryFormat format;
    };
}

#pragma once

#include <Rendering/IShaderProvider.hpp>
#include <Common/IO/FileHelper.hpp>
#include <Common/IO/ShaderReflectionSerializer.hpp>

#include <filesystem>
#include <string>

namespace cp
{
    /**
     * @brief IShaderProvider implementation for standalone runtimes.
     *
     * Resolves a logical shader name (e.g. "Phong") to a pre-compiled
     * binary (.spv or .dxil) by:
     *   1. Looking in the caller-supplied @c _directory first.
     *   2. Looking in the configured @c shaderDir.
     *   3. Falling back to FindFileInParentTree() for development convenience.
     *
     * A reflection sidecar (.refl) is loaded alongside the binary so that
     * Material::Compile() and MaterialInstance::RebuildGpuBuffers() can build
     * the correct descriptor set layout and GPU parameter buffers without
     * re-invoking the shader compiler.
     */
    class PrecompiledShaderProvider final : public cp::IShaderProvider
    {
    public:
        /**
         * @param _shaderDir Directory containing pre-compiled binaries.
         *                   Pass an empty path to rely solely on FindFileInParentTree.
         * @param _format    Binary format of the pre-compiled files.
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

            // 1. Caller-supplied directory
            if (!_directory.empty())
            {
                const std::filesystem::path candidate = _directory / binaryName;

                if (std::filesystem::exists(candidate))
                {
                    resolved = candidate;
                }
            }

            // 2. Configured shader directory
            if (resolved.empty() && !shaderDir.empty())
            {
                const std::filesystem::path candidate = shaderDir / binaryName;

                if (std::filesystem::exists(candidate))
                {
                    resolved = candidate;
                }
            }

            // 3. Parent-tree search (development fallback)
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

            cp::ShaderProviderResult result {
                .success = true,
                .binary = std::move(bytes),
                .format = format,
            };

            // Load the reflection sidecar (.refl) so that Material::Compile() can
            // build the descriptor set layout and MaterialInstance::RebuildGpuBuffers()
            // can pack the correct GPU parameter buffers.
            // A missing sidecar is not fatal here — the caller decides what to do with
            // an empty reflection (Material::Compile will return false and log nothing).
            const std::filesystem::path reflPath = std::filesystem::path(resolved).replace_extension(".refl");
            if (!ReadShaderReflection(reflPath, result.reflection))
            {
                // result.success = false;
                // result.diagnostics = "Could not load the shader reflection sidecar: " + reflPath.stem().string();
            }

            return result;
        }

    private:
        std::filesystem::path shaderDir;
        cp::ShaderBinaryFormat format;
    };
}

#pragma once

#include <Rendering/IShaderProvider.hpp>
#include <ShaderCompiler/ShaderCache.hpp>
#include <ShaderCompiler/ShaderCompiler.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace cp::editor
{
    /**
     * @brief IShaderProvider implementation for the editor.
     *
     * On GetShader(), searches the configured search paths for the requested
     * .slang file, compiles it via Slang, and caches the result on disk.
     * Subsequent calls reuse the cache unless the source file has changed.
     */
    class ShaderCacheProvider final : public cp::IShaderProvider
    {
    public:
        /**
         * @param _cacheDir Directory where compiled binaries are persisted.
         * @param _target Binary format expected by the active RHI.
         * @param _searchPaths Ordered list of directories to search for .slang sources.
         */
        ShaderCacheProvider(
            const std::filesystem::path& _cacheDir,
            cp::ShaderTarget _target,
            std::vector<std::filesystem::path> _searchPaths
        )
            : cache(_cacheDir, _target)
            , format(_target == cp::ShaderTarget::Vulkan_SPIRV
                ? cp::ShaderBinaryFormat::SpirV
                : cp::ShaderBinaryFormat::Dxil)
            , searchPaths(std::move(_searchPaths))
        {}

        [[nodiscard]] cp::ShaderProviderResult GetShader(
            const std::filesystem::path& _shaderName,
            const std::filesystem::path& _directory = {}
        ) override
        {
            const std::filesystem::path sourcePath = FindSource(_shaderName, _directory);

            if (sourcePath.empty())
            {
                return {
                    .success = false,
                    .diagnostics = "Shader source not found: " + _shaderName.string()
                };
            }

            const cp::CompileResult compiled = cache.GetOrCompile(sourcePath, compiler);

            return cp::ShaderProviderResult {
                .success = compiled.success,
                .diagnostics = compiled.diagnostics,
                .binary = compiled.binary,
                .format = format,
                .reflection = compiled.reflection,
            };
        }

    private:
        [[nodiscard]] std::filesystem::path FindSource(
            const std::filesystem::path& _name,
            const std::filesystem::path& _directory = {}
        ) const
        {
            // Ensure the name carries a .slang extension regardless of what was passed
            std::filesystem::path realName = _name;
            if (realName.extension() != ".slang")
                realName.replace_extension(".slang");

            // Absolute path — use directly if it exists
            if (realName.is_absolute() && std::filesystem::exists(realName))
                return realName;

            const std::filesystem::path filename = realName.filename();

            // Caller-supplied directory takes priority
            if (!_directory.empty())
            {
                const std::filesystem::path candidate = _directory / filename;
                if (std::filesystem::exists(candidate))
                    return candidate;
            }

            // Fall back to the provider's own search paths
            for (const auto& dir : searchPaths)
            {
                const std::filesystem::path candidate = dir / filename;
                if (std::filesystem::exists(candidate))
                    return candidate;
            }

            return {};
        }

        cp::ShaderCompiler compiler;
        cp::ShaderCache cache;
        cp::ShaderBinaryFormat format;
        std::vector<std::filesystem::path> searchPaths;
    };
}

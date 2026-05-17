#pragma once

#include "ShaderCompiler.hpp"

#include <filesystem>

namespace cp
{
    /**
     * @brief Timestamp-based on-disk cache for compiled shader binaries.
     *
     * On a cache hit (source file older than the cached binary) the binary is
     * loaded from disk without invoking the compiler.
     * On a cache miss the source is compiled via the provided ShaderCompiler and
     * the result is written to disk for future runs.
     *
     * Cache files are stored as raw SPIR-V / DXIL bytes under @ref cacheDir with
     * the extension @c .spv (Vulkan) or @c .dxil (DX12).
     *
     * @note: Thread-safety: not thread-safe. Guard with a mutex if shared across threads.
     */
    class ShaderCache
    {
    public:
        /**
         * @param _cacheDir Directory where compiled binaries are stored.
         *                  Created automatically on first write.
         * @param _target Target binary format (determines file extension).
         */
        ShaderCache(const std::filesystem::path& _cacheDir, ShaderTarget _target);

        /**
         * @brief Return a compiled binary for @p _sourcePath, reusing the cache when valid.
         *
         * A cached entry is considered valid when the cache file's last-write time
         * is greater than or equal to the source file's last-write time.
         *
         * @param _sourcePath Absolute path to the .slang source file.
         * @param _compiler Compiler instance used on a cache miss.
         * @return CompileResult from cache or from a fresh compile.
         */
        [[nodiscard]] CompileResult GetOrCompile(
            const std::filesystem::path& _sourcePath,
            const ShaderCompiler& _compiler
        );

    private:
        [[nodiscard]] std::filesystem::path GetCachePath(const std::filesystem::path& _sourcePath) const;

        std::filesystem::path cacheDir;
        ShaderTarget target;
    };
}

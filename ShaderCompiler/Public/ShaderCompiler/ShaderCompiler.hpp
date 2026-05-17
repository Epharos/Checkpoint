#pragma once

#include "ShaderReflection.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace cp
{
    /** @brief Target graphics API / binary format for shader compilation. */
    enum class ShaderTarget
    {
        Vulkan_SPIRV,
        DX12_DXIL,
    };

    /** @brief Result returned by every compile operation. */
    struct CompileResult
    {
        /** @brief True if compilation succeeded and @ref binary is valid. */
        bool success = false;

        /** @brief Compiler warnings and errors, empty on a clean build. */
        std::string diagnostics;

        /**
         * @brief Names of the entry points present in @ref binary.
         * Single-entry-point compiles contain exactly one name;
         * CompileAll* compiles list all discovered entry points.
         */
        std::vector<std::string> entryPoints;

        /** @brief Raw SPIR-V or DXIL bytecode. Empty when @ref success is false. */
        std::vector<uint8_t> binary;

        /** @brief Full shader reflection: bindings, struct layouts, field offsets and sizes. */
        ShaderReflection reflection;
    };

    /**
     * @brief Runtime Slang shader compiler.
     *
     * Wraps a persistent Slang global session (expensive to initialise).
     * Create one instance per thread or share behind a mutex.
     * Non-copyable; move semantics are not provided intentionally.
     */
    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        ShaderCompiler(const ShaderCompiler&) = delete;
        ShaderCompiler& operator=(const ShaderCompiler&) = delete;

        /**
         * @brief Compile a single named entry point from a .slang file on disk.
         *
         * The file's parent directory is automatically added to the Slang include
         * search path so that relative @c #include directives inside the shader work.
         *
         * @param _filePath Absolute or relative path to the .slang source file.
         * @param _entryPoint Name of the entry-point function to compile (e.g. @c "MainCS").
         * @param _target Output binary format.
         * @return CompileResult with @c success == true and @c binary populated on success.
         */
        [[nodiscard]] CompileResult CompileFile(
            const std::filesystem::path& _filePath,
            std::string_view _entryPoint,
            ShaderTarget _target
        ) const;

        /**
         * @brief Compile a single named entry point from a Slang source string.
         *
         * @param _source Slang source code.
         * @param _moduleName Virtual module name used in error messages and as the
         *                    implicit file name (e.g. @c "MyShader" : @c "MyShader.slang").
         * @param _entryPoint Name of the entry-point function to compile.
         * @param _target Output binary format.
         * @return CompileResult with @c success == true and @c binary populated on success.
         */
        [[nodiscard]] CompileResult CompileString(
            std::string_view _source,
            std::string_view _moduleName,
            std::string_view _entryPoint,
            ShaderTarget _target
        ) const;

        /**
         * @brief Discover and compile all entry points in a .slang file into a single binary.
         *
         * Uses @c SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM so that all entry points share
         * one SPIR-V / DXIL module. @c result.entryPoints lists their original names.
         *
         * @param _filePath Absolute or relative path to the .slang source file.
         * @param _target Output binary format.
         * @return CompileResult whose @c binary contains the whole-program output.
         */
        [[nodiscard]] CompileResult CompileAllFile(
            const std::filesystem::path& _filePath,
            ShaderTarget _target
        ) const;

        /**
         * @brief Discover and compile all entry points in a Slang source string into a single binary.
         *
         * @param _source Slang source code.
         * @param _moduleName Virtual module name used in error messages.
         * @param _target Output binary format.
         * @return CompileResult whose @c binary contains the whole-program output.
         */
        [[nodiscard]] CompileResult CompileAllString(
            std::string_view _source,
            std::string_view _moduleName,
            ShaderTarget _target
        ) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}

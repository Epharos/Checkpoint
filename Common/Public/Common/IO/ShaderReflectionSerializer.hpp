#pragma once

#include <Common/ShaderReflection.hpp>

#include <filesystem>

namespace cp
{
    /**
     * @brief Write a ShaderReflection to a binary sidecar file (.refl).
     *
     * The file is written in a simple tagged binary format.
     * Call this alongside writing a compiled shader binary so that the reflection data
     * needed to build descriptor set layouts and GPU parameter buffers is available
     * without re-invoking the shader compiler at load time.
     *
     * @param _path       Destination path for the sidecar file (e.g. "Phong.refl").
     * @param _reflection Reflection to serialise.
     */
    void WriteShaderReflection(const std::filesystem::path& _path, const ShaderReflection& _reflection);

    /**
     * @brief Read a ShaderReflection from a binary sidecar file (.refl).
     *
     * @param _path       Path of the sidecar file to read.
     * @param _reflection Output reflection, populated on success.
     * @return true on success; false if the file is missing, corrupt, or has an
     *         incompatible version.
     */
    [[nodiscard]] bool ReadShaderReflection(const std::filesystem::path& _path, ShaderReflection& _reflection);
}

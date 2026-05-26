#include <ShaderCompiler/ShaderCache.hpp>

#include <Common/Core/ProjectContext.hpp>
#include <Common/IO/ShaderReflectionSerializer.hpp>

#include <fstream>

namespace cp
{
    ShaderCache::ShaderCache(const std::filesystem::path& _cacheDir, const ShaderTarget _target)
        : cacheDir(_cacheDir), target(_target) {}

    CompileResult ShaderCache::GetOrCompile(
        const std::filesystem::path& _sourcePath,
        const ShaderCompiler& _compiler
    )
    {
        namespace fs = std::filesystem;

        const fs::path cachePath = GetCachePath(_sourcePath);
        const fs::path reflPath = fs::path(cachePath).replace_extension(".refl");

        // Cache hit
        if (fs::exists(cachePath) && fs::exists(reflPath) && fs::exists(_sourcePath))
        {
            std::error_code ec;
            const auto sourceTime = fs::last_write_time(_sourcePath, ec);
            const auto cacheTime = fs::last_write_time(cachePath, ec);

            if (!ec && cacheTime >= sourceTime)
            {
                std::ifstream file(cachePath, std::ios::binary | std::ios::ate);

                if (file.is_open())
                {
                    const auto size = static_cast<size_t>(file.tellg());
                    file.seekg(0, std::ios::beg);

                    CompileResult result;
                    result.binary.resize(size);
                    file.read(reinterpret_cast<char*>(result.binary.data()), static_cast<std::streamsize>(size));
                    result.success = file.good();

                    if (result.success)
                    {
                        result.success = ReadShaderReflection(reflPath, result.reflection);
                    }

                    if (result.success)
                    {
                        return result;
                    }
                }
            }
        }

        // Cache miss or corrupt
        CompileResult result = _compiler.CompileAllFile(_sourcePath, target);

        if (result.success && !result.binary.empty())
        {
            std::error_code ec;
            fs::create_directories(cachePath.parent_path(), ec);

            std::ofstream file(cachePath, std::ios::binary);

            if (file.is_open())
            {
                file.write(
                    reinterpret_cast<const char*>(result.binary.data()),
                    static_cast<std::streamsize>(result.binary.size())
                );
            }

            WriteShaderReflection(reflPath, result.reflection);
        }

        return result;
    }

    std::filesystem::path ShaderCache::GetCachePath(const std::filesystem::path& _sourcePath) const
    {
        const std::string ext = (target == ShaderTarget::Vulkan_SPIRV) ? ".spv" : ".dxil";

        const std::filesystem::path& projectRoot = GetProjectRootPath();

        std::filesystem::path relPath;

        if (!projectRoot.empty())
        {
            std::error_code ec;
            relPath = std::filesystem::relative(_sourcePath, projectRoot, ec);

            if (ec || relPath.empty() || *relPath.begin() == "..")
            {
                relPath = _sourcePath.filename();
            }
        }
        else
        {
            relPath = _sourcePath.filename();
        }

        relPath.replace_extension(ext);
        return cacheDir / relPath;
    }
}

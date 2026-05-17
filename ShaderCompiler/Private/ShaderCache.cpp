#include <ShaderCompiler/ShaderCache.hpp>

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

        // Cache hit: cache file exists and is at least as recent as the source
        if (fs::exists(cachePath) && fs::exists(_sourcePath))
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
                    return result;
                }
            }
        }

        // Cache miss: compile and persist
        CompileResult result = _compiler.CompileAllFile(_sourcePath, target);

        if (result.success && !result.binary.empty())
        {
            std::error_code ec;
            fs::create_directories(cacheDir, ec);

            std::ofstream file(cachePath, std::ios::binary);
            if (file.is_open())
            {
                file.write(
                    reinterpret_cast<const char*>(result.binary.data()),
                    static_cast<std::streamsize>(result.binary.size())
                );
            }
        }

        return result;
    }

    std::filesystem::path ShaderCache::GetCachePath(const std::filesystem::path& _sourcePath) const
    {
        const std::string ext = (target == ShaderTarget::Vulkan_SPIRV) ? ".spv" : ".dxil";
        return cacheDir / (_sourcePath.stem().string() + ext);
    }
}

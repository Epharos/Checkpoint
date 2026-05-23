#include <ShaderCompiler/ShaderCache.hpp>

#include <Common/Core/ProjectContext.hpp>

#include <fstream>

namespace cp
{
    static constexpr uint32_t ReflMagic = 0x52464C58u; // 'RFLX'
    static constexpr uint32_t ReflVersion = 1u;

    static void WriteU8(std::ofstream& f, uint8_t v)
    {
        f.write(reinterpret_cast<const char*>(&v), 1);
    }

    static void WriteU32(std::ofstream& f, uint32_t v)
    {
        f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }

    static void WriteString(std::ofstream& f, const std::string& s)
    {
        const auto len = static_cast<uint32_t>(s.size());
        WriteU32(f, len);

        if (len > 0)
        {
            f.write(s.data(), len);
        }
    }

    static bool ReadU8(std::ifstream& f, uint8_t& v)
    {
        f.read(reinterpret_cast<char*>(&v), 1);
        return f.good();
    }

    static bool ReadU32(std::ifstream& f, uint32_t& v)
    {
        f.read(reinterpret_cast<char*>(&v), sizeof(v));
        return f.good();
    }

    static bool ReadString(std::ifstream& f, std::string& s)
    {
        uint32_t len = 0;

        if (!ReadU32(f, len))
        {
            return false;
        }

        s.resize(len);

        if (len > 0)
        {
            f.read(s.data(), len);
        }

        return f.good();
    }

    static void WriteField(std::ofstream& f, const FieldReflection& field);

    static void WriteField(std::ofstream& f, const FieldReflection& field)
    {
        WriteString(f, field.name);
        WriteString(f, field.typeName);
        WriteU8(f, static_cast<uint8_t>(field.kind));
        WriteU8(f, static_cast<uint8_t>(field.scalar));
        WriteU32(f, field.rows);
        WriteU32(f, field.columns);
        WriteU32(f, field.offset);
        WriteU32(f, field.size);
        WriteU32(f, field.arrayCount);
        WriteU32(f, field.stride);

        WriteU32(f, static_cast<uint32_t>(field.annotations.size()));

        for (const auto& a : field.annotations)
        {
            WriteString(f, a);
        }

        WriteU32(f, static_cast<uint32_t>(field.fields.size()));

        for (const auto& nested : field.fields)
        {
            WriteField(f, nested);
        }
    }

    static bool ReadField(std::ifstream& f, FieldReflection& field)
    {
        uint8_t kind = 0;
        uint8_t scalar = 0;

        if (!ReadString(f, field.name))
        {
            return false;
        }

        if (!ReadString(f, field.typeName))
        {
            return false;
        }

        if (!ReadU8(f, kind))
        {
            return false;
        }

        if (!ReadU8(f, scalar))
        {
            return false;
        }

        field.kind = static_cast<FieldKind>(kind);
        field.scalar = static_cast<ScalarKind>(scalar);

        if (!ReadU32(f, field.rows))
        {
            return false;
        }

        if (!ReadU32(f, field.columns))
        {
            return false;
        }

        if (!ReadU32(f, field.offset))
        {
            return false;
        }

        if (!ReadU32(f, field.size))
        {
            return false;
        }

        if (!ReadU32(f, field.arrayCount))
        {
            return false;
        }

        if (!ReadU32(f, field.stride))
        {
            return false;
        }

        uint32_t annotCount = 0;

        if (!ReadU32(f, annotCount))
        {
            return false;
        }

        field.annotations.resize(annotCount);

        for (auto& a : field.annotations)
        {
            if (!ReadString(f, a))
            {
                return false;
            }
        }

        uint32_t nestedCount = 0;

        if (!ReadU32(f, nestedCount))
        {
            return false;
        }

        field.fields.resize(nestedCount);

        for (auto& nested : field.fields)
        {
            if (!ReadField(f, nested))
            {
                return false;
            }
        }

        return true;
    }

    static void WriteReflection(const std::filesystem::path& path, const ShaderReflection& refl)
    {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);

        if (!f.is_open())
        {
            return;
        }

        WriteU32(f, ReflMagic);
        WriteU32(f, ReflVersion);
        WriteU32(f, static_cast<uint32_t>(refl.bindings.size()));

        for (const auto& b : refl.bindings)
        {
            WriteString(f, b.name);
            WriteString(f, b.typeName);
            WriteU8(f, static_cast<uint8_t>(b.kind));
            WriteU32(f, b.binding);
            WriteU32(f, b.set);
            WriteU32(f, b.size);
            WriteU32(f, b.stride);

            WriteU32(f, static_cast<uint32_t>(b.annotations.size()));

            for (const auto& a : b.annotations)
            {
                WriteString(f, a);
            }

            WriteU32(f, static_cast<uint32_t>(b.fields.size()));

            for (const auto& field : b.fields)
            {
                WriteField(f, field);
            }
        }
    }

    static bool ReadReflection(const std::filesystem::path& path, ShaderReflection& refl)
    {
        std::ifstream f(path, std::ios::binary);

        if (!f.is_open())
        {
            return false;
        }

        uint32_t magic = 0;
        uint32_t version = 0;

        if (!ReadU32(f, magic) || magic != ReflMagic)
        {
            return false;
        }

        if (!ReadU32(f, version) || version != ReflVersion)
        {
            return false;
        }

        uint32_t bindingCount = 0;

        if (!ReadU32(f, bindingCount))
        {
            return false;
        }

        refl.bindings.resize(bindingCount);

        for (auto& b : refl.bindings)
        {
            uint8_t kind = 0;

            if (!ReadString(f, b.name))
            {
                return false;
            }

            if (!ReadString(f, b.typeName))
            {
                return false;
            }

            if (!ReadU8(f, kind))
            {
                return false;
            }

            b.kind = static_cast<BindingKind>(kind);

            if (!ReadU32(f, b.binding))
            {
                return false;
            }

            if (!ReadU32(f, b.set))
            {
                return false;
            }

            if (!ReadU32(f, b.size))
            {
                return false;
            }

            if (!ReadU32(f, b.stride))
            {
                return false;
            }

            uint32_t annotCount = 0;

            if (!ReadU32(f, annotCount))
            {
                return false;
            }

            b.annotations.resize(annotCount);

            for (auto& a : b.annotations)
            {
                if (!ReadString(f, a))
                {
                    return false;
                }
            }

            uint32_t fieldCount = 0;

            if (!ReadU32(f, fieldCount))
            {
                return false;
            }

            b.fields.resize(fieldCount);

            for (auto& field : b.fields)
            {
                if (!ReadField(f, field))
                {
                    return false;
                }
            }
        }

        return f.good() || f.eof();
    }

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
                        result.success = ReadReflection(reflPath, result.reflection);
                    }

                    if (result.success)
                    {
                        return result;
                    }
                }
            }
        }

        // Cache miss or corrupted
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

            WriteReflection(reflPath, result.reflection);
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

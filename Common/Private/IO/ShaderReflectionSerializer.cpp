#include <Common/IO/ShaderReflectionSerializer.hpp>

#include <fstream>

namespace cp
{
    namespace
    {
        static constexpr uint32_t ReflMagic = 0x52464C58u; // 'RFLX'
        static constexpr uint32_t ReflVersion = 1u;

        void WriteU8(std::ofstream& f, uint8_t v)
        {
            f.write(reinterpret_cast<const char*>(&v), 1);
        }

        void WriteU32(std::ofstream& f, uint32_t v)
        {
            f.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }

        void WriteStr(std::ofstream& f, const std::string& s)
        {
            const auto len = static_cast<uint32_t>(s.size());
            WriteU32(f, len);

            if (len > 0)
            {
                f.write(s.data(), len);
            }
        }

        bool ReadU8(std::ifstream& f, uint8_t& v)
        {
            f.read(reinterpret_cast<char*>(&v), 1);
            return f.good();
        }

        bool ReadU32(std::ifstream& f, uint32_t& v)
        {
            f.read(reinterpret_cast<char*>(&v), sizeof(v));
            return f.good();
        }

        bool ReadStr(std::ifstream& f, std::string& s)
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

        void WriteField(std::ofstream& f, const FieldReflection& field)
        {
            WriteStr(f, field.name);
            WriteStr(f, field.typeName);
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
                WriteStr(f, a);
            }

            WriteU32(f, static_cast<uint32_t>(field.fields.size()));

            for (const auto& nested : field.fields)
            {
                WriteField(f, nested);
            }
        }

        bool ReadField(std::ifstream& f, FieldReflection& field)
        {
            uint8_t kind = 0;
            uint8_t scalar = 0;

            if (!ReadStr(f, field.name))
            {
                return false;
            }

            if (!ReadStr(f, field.typeName))
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
                if (!ReadStr(f, a))
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
    }

    void WriteShaderReflection(const std::filesystem::path& _path, const ShaderReflection& _reflection)
    {
        std::ofstream f(_path, std::ios::binary | std::ios::trunc);

        if (!f.is_open())
        {
            return;
        }

        WriteU32(f, ReflMagic);
        WriteU32(f, ReflVersion);
        WriteU32(f, static_cast<uint32_t>(_reflection.bindings.size()));

        for (const auto& b : _reflection.bindings)
        {
            WriteStr(f, b.name);
            WriteStr(f, b.typeName);
            WriteU8(f, static_cast<uint8_t>(b.kind));
            WriteU32(f, b.binding);
            WriteU32(f, b.set);
            WriteU32(f, b.size);
            WriteU32(f, b.stride);

            WriteU32(f, static_cast<uint32_t>(b.annotations.size()));

            for (const auto& a : b.annotations)
            {
                WriteStr(f, a);
            }

            WriteU32(f, static_cast<uint32_t>(b.fields.size()));

            for (const auto& field : b.fields)
            {
                WriteField(f, field);
            }
        }
    }

    bool ReadShaderReflection(const std::filesystem::path& _path, ShaderReflection& _reflection)
    {
        std::ifstream f(_path, std::ios::binary);

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

        _reflection.bindings.resize(bindingCount);

        for (auto& b : _reflection.bindings)
        {
            uint8_t kind = 0;

            if (!ReadStr(f, b.name))
            {
                return false;
            }

            if (!ReadStr(f, b.typeName))
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
                if (!ReadStr(f, a))
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
}

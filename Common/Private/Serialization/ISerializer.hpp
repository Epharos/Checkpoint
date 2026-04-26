#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>

namespace cp
{
    class ISerializer
    {
    public:
        virtual ~ISerializer() = default;

        virtual void Write(std::span<const std::byte> _bytes) = 0;

        template<typename T>
        requires std::is_trivially_copyable_v<T>
        void WritePod(const T& _value)
        {
            Write(std::as_bytes(std::span<const T>(&_value, 1)));
        }

        void WriteString(const std::string& _value)
        {
            const auto size = static_cast<uint32_t>(_value.size());
            WritePod(size);

            if (size == 0)
            {
                return;
            }

            Write(std::as_bytes(std::span<const char>(_value.data(), _value.size())));
        }
    };

    class IDeserializer
    {
    public:
        virtual ~IDeserializer() = default;

        [[nodiscard]] virtual bool Read(std::span<std::byte> _bytes) = 0;

        template<typename T>
        requires std::is_trivially_copyable_v<T>
        [[nodiscard]] bool ReadPod(T& _value)
        {
            return Read(std::as_writable_bytes(std::span<T>(&_value, 1)));
        }

        [[nodiscard]] bool ReadString(std::string& _value)
        {
            uint32_t size = 0;
            if (!ReadPod(size))
            {
                return false;
            }

            _value.resize(size);
            if (size == 0)
            {
                return true;
            }

            return Read(std::as_writable_bytes(std::span<char>(_value.data(), _value.size())));
        }
    };
}

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace cp::ecs
{
    using TypeGuid = uint64_t;

    /**
     * @brief Stable compile-time 64-bit FNV-1a hash for canonical type names.
     */
    [[nodiscard]] constexpr TypeGuid MakeTypeGuid(const std::string_view _canonicalName)
    {
        constexpr TypeGuid offset = 14695981039346656037ull;
        constexpr TypeGuid prime = 1099511628211ull;

        TypeGuid hash = offset;
        for (const char c : _canonicalName)
        {
            hash ^= static_cast<uint8_t>(c);
            hash *= prime;
        }

        return hash;
    }

    /**
     * @brief Converts a guid to a fixed-width hex string for registry keys.
     */
    [[nodiscard]] inline std::string GuidToRegistryKey(const TypeGuid _guid)
    {
        constexpr std::array<char, 17> hex =
            {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', '\0'};

        std::string key(16, '0');
        for (size_t i = 0; i < 16; ++i)
        {
            const auto nibble = static_cast<uint8_t>((_guid >> ((15 - i) * 4)) & 0xF);
            key[i] = hex[nibble];
        }

        return key;
    }
}

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace cp
{
    inline std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& _path)
    {
        std::ifstream ifs(_path, std::ios::binary | std::ios::ate);

        if (!ifs.is_open())
        {
            return {};
        }

        std::vector<uint8_t> bytes;
        bytes.resize(ifs.tellg());

        ifs.seekg(0);
        ifs.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
        ifs.close();

        return bytes;
    }
}

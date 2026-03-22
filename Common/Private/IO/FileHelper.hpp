#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace cp
{
    inline std::vector<uint8_t> LoadBinaryFile(const std::filesystem::path& _path)
    {
        std::ifstream ifs(_path, std::ios::binary | std::ios::ate);

        if (!ifs.is_open())
        {
            return {};
        }

        std::vector<uint8_t> bytes;
        bytes.resize(ifs.tellg());

        ifs.seekg(0, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
        ifs.close();

        return bytes;
    }

    inline std::filesystem::path FindFileInParentTree(const std::string_view _fileName)
    {
        std::filesystem::path currentPath = std::filesystem::current_path();
        const std::filesystem::path fileNamePath(_fileName);

        while (!currentPath.empty())
        {
            const std::filesystem::path candidatePath = currentPath / fileNamePath;
            if (std::filesystem::exists(candidatePath))
            {
                return candidatePath;
            }

            const std::filesystem::path parentPath = currentPath.parent_path();
            if (parentPath == currentPath)
            {
                break;
            }

            currentPath = parentPath;
        }

        return {};
    }
}

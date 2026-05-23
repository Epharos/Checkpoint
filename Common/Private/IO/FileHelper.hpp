#pragma once

#include <Common/Core/ProjectContext.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <system_error>
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
        const std::filesystem::path& projectRoot = GetProjectRootPath();
        std::filesystem::path currentPath = projectRoot.empty() ? std::filesystem::current_path() : projectRoot;
        const std::filesystem::path fileNamePath(_fileName);

        while (!currentPath.empty())
        {
            const std::filesystem::path candidatePath = currentPath / fileNamePath;
            if (std::filesystem::exists(candidatePath))
            {
                return std::filesystem::weakly_canonical(candidatePath);
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

    inline std::filesystem::path FindFileFromDirectory(
        const std::string_view _fileName,
        const std::filesystem::path& _baseDir
    )
    {
        if (!_baseDir.empty())
        {
            const std::filesystem::path candidate = _baseDir / _fileName;
            if (std::filesystem::exists(candidate))
            {
                return std::filesystem::weakly_canonical(candidate);
            }
        }

        return FindFileInParentTree(_fileName);
    }

    inline std::filesystem::path ResolveAssetPath(const std::string_view _path)
    {
        if (_path.empty())
        {
            return {};
        }

        const std::filesystem::path p(_path);

        if (p.is_absolute())
        {
            return std::filesystem::exists(p) ? std::filesystem::weakly_canonical(p) : p;
        }

        return FindFileInParentTree(_path);
    }

    inline std::filesystem::path GetRelativePath(
        const std::filesystem::path& _path,
        const std::filesystem::path& _currentPath = {}
    )
    {
        if (_path.empty())
        {
            return {};
        }

        const std::filesystem::path& projectRoot = GetProjectRootPath();
        const std::filesystem::path& effectiveBase = !_currentPath.empty()
            ? _currentPath
            : (!projectRoot.empty() ? projectRoot : std::filesystem::current_path());

        std::filesystem::path absolutePath = std::filesystem::absolute(_path);

        std::error_code ec;
        std::filesystem::path relativePath = std::filesystem::relative(absolutePath, effectiveBase, ec);
        if (ec || relativePath.empty())
        {
            relativePath = absolutePath.lexically_relative(effectiveBase);
            if (relativePath.empty())
            {
                return absolutePath;
            }
        }

        return relativePath;
    }
}

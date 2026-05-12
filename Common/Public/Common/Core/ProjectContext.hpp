#pragma once

#include <filesystem>

namespace cp
{
    inline std::filesystem::path& GetProjectRootPathRef()
    {
        static std::filesystem::path s_projectRootPath;
        return s_projectRootPath;
    }

    inline const std::filesystem::path& GetProjectRootPath()
    {
        return GetProjectRootPathRef();
    }

    inline void SetProjectRootPath(std::filesystem::path _path)
    {
        GetProjectRootPathRef() = std::move(_path);
    }
}

#pragma once

#include <filesystem>

namespace cp
{
    inline std::filesystem::path examplePluginDir;

    inline void SetExamplePluginDir(std::filesystem::path _dir)
    {
        examplePluginDir = std::move(_dir);
    }

    inline const std::filesystem::path& GetExamplePluginDir()
    {
        return examplePluginDir;
    }
}

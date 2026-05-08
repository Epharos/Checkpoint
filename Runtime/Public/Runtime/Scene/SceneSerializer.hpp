#pragma once

#include <filesystem>
#include <memory>

namespace cp::runtime
{
    class Scene;
}

namespace cp
{
    class ILogger;
}

namespace cp::runtime
{
    class SceneSerializer
    {
    public:
        [[nodiscard]] static std::unique_ptr<Scene> LoadSceneFromFile(
            const std::filesystem::path& _filePath,
            cp::ILogger* _logger = nullptr
        );

        [[nodiscard]] static bool SaveSceneToFile(
            const Scene& _scene,
            const std::filesystem::path& _filePath,
            cp::ILogger* _logger = nullptr
        );
    };
}

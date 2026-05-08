#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cp
{
    class ILogger;
}

namespace cp::runtime
{
    class Scene;

    using SceneChangeCallback = std::function<void(Scene* _previous, Scene* _next)>;

    class SceneManager
    {
    public:
        explicit SceneManager(cp::ILogger* _logger = nullptr);
        ~SceneManager();

        void SetScenesDirectory(const std::filesystem::path& _dir);
        [[nodiscard]] std::filesystem::path GetScenesDirectory() const;

        [[nodiscard]] Scene* LoadScene(const std::filesystem::path& _sceneFilePath);
        [[nodiscard]] Scene* LoadSceneByName(const std::string& _sceneName);
        [[nodiscard]] Scene* CreateEmptyScene(const std::string& _name);

        [[nodiscard]] Scene* GetActiveScene();
        [[nodiscard]] const Scene* GetActiveScene() const;

        void UnloadActiveScene();

        [[nodiscard]] bool SaveActiveScene() const;
        [[nodiscard]] bool SaveSceneAs(const std::filesystem::path& _newPath);

        void OnSceneChanged(SceneChangeCallback _callback);

    private:
        void ActivateScene(
            std::unique_ptr<Scene> _scene,
            const std::filesystem::path& _path = {}
        );

        std::unique_ptr<Scene> activeScene;
        std::filesystem::path scenesDirectory;
        std::filesystem::path activeScenePath;
        std::vector<SceneChangeCallback> sceneChangeCallbacks;
        cp::ILogger* logger;
    };
}

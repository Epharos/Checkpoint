#include <Runtime/Scene/SceneManager.hpp>
#include <Runtime/Scene/Scene.hpp>
#include <Runtime/Scene/SceneSerializer.hpp>

#include <Common/Core/Log.hpp>

namespace cp::runtime
{
    SceneManager::SceneManager(cp::ILogger* _logger) : logger(_logger) {}

    SceneManager::~SceneManager() = default;

    void SceneManager::SetScenesDirectory(const std::filesystem::path& _dir)
    {
        scenesDirectory = _dir;
    }

    std::filesystem::path SceneManager::GetScenesDirectory() const
    {
        return scenesDirectory;
    }

    Scene* SceneManager::LoadScene(const std::filesystem::path& _sceneFilePath)
    {
        auto loaded = SceneSerializer::LoadSceneFromFile(_sceneFilePath, logger);
        if (!loaded)
            return nullptr;

        ActivateScene(std::move(loaded), _sceneFilePath);
        return activeScene.get();
    }

    Scene* SceneManager::LoadSceneByName(const std::string& _sceneName)
    {
        return LoadScene(scenesDirectory / (_sceneName + ".scene"));
    }

    Scene* SceneManager::CreateEmptyScene(const std::string& _name)
    {
        auto scene = std::make_unique<Scene>();
        scene->Initialize(_name);
        ActivateScene(std::move(scene));
        return activeScene.get();
    }

    Scene* SceneManager::GetActiveScene()
    {
        return activeScene.get();
    }

    const Scene* SceneManager::GetActiveScene() const
    {
        return activeScene.get();
    }

    void SceneManager::UnloadActiveScene()
    {
        ActivateScene(nullptr);
        activeScenePath.clear();
    }

    bool SceneManager::SaveActiveScene() const
    {
        if (!activeScene || activeScenePath.empty())
            return false;

        return SceneSerializer::SaveSceneToFile(*activeScene, activeScenePath, logger);
    }

    bool SceneManager::SaveSceneAs(const std::filesystem::path& _newPath)
    {
        if (!activeScene)
            return false;

        if (!SceneSerializer::SaveSceneToFile(*activeScene, _newPath, logger))
            return false;

        activeScenePath = _newPath;
        return true;
    }

    void SceneManager::OnSceneChanged(SceneChangeCallback _callback)
    {
        sceneChangeCallbacks.emplace_back(std::move(_callback));
    }

    void SceneManager::ActivateScene(
        std::unique_ptr<Scene> _scene,
        const std::filesystem::path& _path
    )
    {
        Scene* previous = activeScene.get();
        Scene* next = _scene.get();

        activeScene = std::move(_scene);
        activeScenePath = _path;

        for (auto& cb : sceneChangeCallbacks)
            cb(previous, next);
    }
}

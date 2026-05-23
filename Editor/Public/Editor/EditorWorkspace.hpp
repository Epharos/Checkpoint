#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <EditorUI/EditorUI.hpp>

#include <Common/Plugin/IEditorStateProvider.hpp>
#include <Common/Plugin/IViewportController.hpp>
#include <Common/Plugin/IKeybindRegistrar.hpp>
#include <Common/Plugin/AssetAuthoringRegistrar.hpp>
#include <Common/Plugin/AssetContextMenuRegistrar.hpp>

namespace cp
{
	class ILogger;
	class IShaderProvider;
	class PluginHost;
	class RenderingHardwareInterface;
	class Renderer;
	class RegistryManager;
	class ShaderCompiler;
}

namespace cp::editor
{
	class KeybindRegistry;
	class UserPluginManager;
}

namespace cp::runtime
{
	class Scene;
	class RuntimeEditorApplication;
}

namespace cp::editor
{
	struct WorkspaceConfig
	{
		std::filesystem::path projectRootPath;
		std::filesystem::path executablePath;
	};

	class EditorWorkspace
	{
	public:
		explicit EditorWorkspace(std::shared_ptr<cp::editorui::IEditorUIBackend> _backend);
		~EditorWorkspace();

		void BuildDefaultLayout(const WorkspaceConfig& _config);

	private:
		void AppendConsoleEntry(cp::editorui::ConsoleEntry _entry);
		void OnRuntimeStopped();
		void RefreshPluginManagerView();
		void RefreshSceneConfigView() const;

		std::shared_ptr<cp::editorui::IEditorUIBackend> backend;
		std::shared_ptr<cp::editorui::IApplicationWindow> window;
		std::shared_ptr<cp::editorui::IActionRegistry> actions;
		std::shared_ptr<cp::editorui::ICommandStack> commandStack;
		std::shared_ptr<cp::editorui::IDockHost> dockHost;
		std::shared_ptr<cp::editorui::ISceneHierarchyView> hierarchyView;
		std::shared_ptr<cp::editorui::IAssetExplorerView> assetView;
		std::shared_ptr<cp::editorui::IConsoleView> consoleView;
		std::shared_ptr<cp::editorui::IInspectorView> inspectorView;
		std::shared_ptr<cp::editorui::ISceneConfigView> sceneConfigView;
		std::shared_ptr<cp::editorui::IViewportWidget> viewportView;
		std::shared_ptr<cp::editorui::IPluginManagerView> pluginManagerView;

		std::vector<cp::editorui::ConsoleEntry> consoleEntries;
		std::vector<cp::editorui::ConsoleEntry> pendingConsoleEntries;
		std::mutex pendingEntriesMutex;
		std::thread::id mainThreadId;
		std::shared_ptr<cp::ILogger> logger;
		std::unique_ptr<cp::PluginHost> pluginHost;

		std::unique_ptr<cp::RegistryManager> registryManager;
		std::unique_ptr<cp::runtime::Scene> scene;
		std::unique_ptr<cp::RenderingHardwareInterface> rhi;
		std::unique_ptr<cp::IShaderProvider> shaderProvider;
		std::unique_ptr<cp::Renderer> renderer;
		bool assetRegistryInitialized = false;

		// Runtime play mode
		std::unique_ptr<cp::runtime::RuntimeEditorApplication> runtimeApp;
		std::filesystem::path runtimeBackupPath;
		std::shared_ptr<cp::editorui::IAction> runAction;
		std::shared_ptr<cp::editorui::IAction> stopAction;
		std::function<bool(const std::filesystem::path&)> loadSceneDelegate;

		std::unique_ptr<KeybindRegistry> keybindRegistry;
		std::filesystem::path keybindsConfigPath;

		std::unique_ptr<cp::IEditorStateProvider> editorStateProvider;
		std::unique_ptr<cp::IViewportController> viewportController;
		std::unique_ptr<cp::IKeybindRegistrar> keybindRegistrar;
		std::unique_ptr<cp::ShaderCompiler> reflectionCompiler;
		std::unordered_map<std::string, std::shared_ptr<cp::IAssetAuthoring>> assetAuthoringMap;
		std::unique_ptr<cp::IAssetAuthoringRegistrar> assetAuthoringRegistrar;
		std::unordered_map<std::string, std::shared_ptr<cp::IAssetContextMenuContributor>> assetContextMenuMap;
		std::unique_ptr<cp::IAssetContextMenuRegistrar> assetContextMenuRegistrar;

		std::unique_ptr<cp::editor::UserPluginManager> userPluginManager;
	};
}

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <EditorUI/EditorUI.hpp>

namespace cp
{
	class ILogger;
	class PluginHost;
	class RenderingHardwareInterface;
	class Renderer;
	class RegistryManager;
}

namespace cp::runtime
{
	class Scene;
}

namespace cp::editor
{
	struct WorkspaceConfig
	{
		std::string windowTitle = "Checkpoint Editor";
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

		std::vector<cp::editorui::ConsoleEntry> consoleEntries;
		std::shared_ptr<cp::ILogger> logger;
		std::unique_ptr<cp::PluginHost> pluginHost;

		std::unique_ptr<cp::RegistryManager> registryManager;
		std::unique_ptr<cp::runtime::Scene> scene;
		std::unique_ptr<cp::RenderingHardwareInterface> rhi;
		std::unique_ptr<cp::Renderer> renderer;
		bool assetRegistryInitialized = false;
	};
}

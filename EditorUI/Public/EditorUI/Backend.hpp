#pragma once

#include <memory>

#include <EditorUI/Containers.hpp>

namespace cp::editorui
{
	class IPanelFactory
	{
	public:
		virtual ~IPanelFactory() = default;
		virtual std::shared_ptr<IDockPanel> CreatePanel(IDockHost& _dockHost) = 0;
	};

	class IMenuContributor
	{
	public:
		virtual ~IMenuContributor() = default;
		virtual void Contribute(IMenuBar& _menuBar, IActionRegistry& _actionRegistry) = 0;
	};

	class ICommandContributor
	{
	public:
		virtual ~ICommandContributor() = default;
		virtual void Contribute(IActionRegistry& _actionRegistry, ICommandStack& _commandStack) = 0;
	};

	class IViewportOverlayContributor
	{
	public:
		virtual ~IViewportOverlayContributor() = default;
		virtual std::string_view GetOverlayId() const = 0;
		virtual void Attach(IViewportWidget& _viewport) = 0;
		virtual void Detach(IViewportWidget& _viewport) = 0;
	};

	class IEditorUIBackend
	{
	public:
		virtual ~IEditorUIBackend() = default;

		virtual std::shared_ptr<IApplicationWindow> CreateApplicationWindow(const ApplicationWindowConfig& _config) = 0;
		virtual std::shared_ptr<IActionRegistry> CreateActionRegistry() = 0;
		virtual std::shared_ptr<ICommandStack> CreateCommandStack() = 0;
		virtual std::shared_ptr<ITheme> CreateTheme(std::string _name) = 0;

		virtual std::shared_ptr<ISceneHierarchyView> CreateSceneHierarchyView(std::string _id) = 0;
		virtual std::shared_ptr<IAssetExplorerView> CreateAssetExplorerView(std::string _id) = 0;
		virtual std::shared_ptr<IConsoleView> CreateConsoleView(std::string _id) = 0;
		virtual std::shared_ptr<IInspectorView> CreateInspectorView(std::string _id) = 0;
		virtual std::shared_ptr<IViewportWidget> CreateViewportWidget(const ViewportCreateInfo& _createInfo) = 0;
		virtual std::shared_ptr<ISearchBox> CreateSearchBox(std::string _id) = 0;
		virtual std::shared_ptr<ISceneConfigView> CreateSceneConfigView(std::string _id) = 0;
		virtual std::shared_ptr<IContextMenu> CreateContextMenu(std::string _id) = 0;
		virtual std::shared_ptr<IFileDialog> CreateFileDialog(std::string _id) = 0;
	};
}

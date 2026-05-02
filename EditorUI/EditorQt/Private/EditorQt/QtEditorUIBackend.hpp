#pragma once

#include "EditorQtCommon.hpp"
#include "QtActionRegistry.hpp"
#include "QtApplicationWindow.hpp"
#include "QtAssetExplorerView.hpp"
#include "QtCommandStack.hpp"
#include "QtConsoleView.hpp"
#include "QtContextMenu.hpp"
#include "QtFileDialog.hpp"
#include "QtInspectorView.hpp"
#include "QtSceneHierarchyView.hpp"
#include "QtSearchBox.hpp"
#include "QtTheme.hpp"
#include "QtViewportWidget.hpp"

namespace cp::editorqt
{
	class QtEditorUIBackend final : public cp::editorui::IEditorUIBackend
	{
	public:
		QtEditorUIBackend()
		{
			if (QApplication::instance() == nullptr)
			{
				static int argc = 0;
				static std::vector<char*> argv = { nullptr };
				ownedApplication = std::make_unique<QApplication>(argc, argv.data());
			}
		}

		std::shared_ptr<cp::editorui::IApplicationWindow> CreateApplicationWindow(
			const cp::editorui::ApplicationWindowConfig& _config) override
		{
			return std::make_shared<QtApplicationWindow>(_config);
		}

		std::shared_ptr<cp::editorui::IActionRegistry> CreateActionRegistry() override
		{
			return std::make_shared<QtActionRegistry>();
		}

		std::shared_ptr<cp::editorui::ICommandStack> CreateCommandStack() override
		{
			return std::make_shared<QtCommandStack>();
		}

		std::shared_ptr<cp::editorui::ITheme> CreateTheme(std::string _name) override
		{
			return std::make_shared<QtTheme>(std::move(_name));
		}

		std::shared_ptr<cp::editorui::ISceneHierarchyView> CreateSceneHierarchyView(std::string _id) override
		{
			return std::make_shared<QtSceneHierarchyView>(std::move(_id));
		}

		std::shared_ptr<cp::editorui::IAssetExplorerView> CreateAssetExplorerView(std::string _id) override
		{
			return std::make_shared<QtAssetExplorerView>(std::move(_id));
		}

		std::shared_ptr<cp::editorui::IConsoleView> CreateConsoleView(std::string _id) override
		{
			return std::make_shared<QtConsoleView>(std::move(_id));
		}

		std::shared_ptr<cp::editorui::IInspectorView> CreateInspectorView(std::string _id) override
		{
			return std::make_shared<QtInspectorView>(std::move(_id));
		}

		std::shared_ptr<cp::editorui::IViewportWidget> CreateViewportWidget(
			const cp::editorui::ViewportCreateInfo& _createInfo) override
		{
			return std::make_shared<QtViewportWidget>(_createInfo.id);
		}

		std::shared_ptr<cp::editorui::ISearchBox> CreateSearchBox(std::string _id) override
		{
			return std::make_shared<QtSearchBox>(std::move(_id));
		}

		std::shared_ptr<cp::editorui::IContextMenu> CreateContextMenu(std::string _id) override
		{
			return std::make_shared<QtContextMenu>(_id);
		}

		std::shared_ptr<cp::editorui::IFileDialog> CreateFileDialog(std::string _id) override
		{
			return std::make_shared<QtFileDialog>(std::move(_id));
		}

	private:
		std::unique_ptr<QApplication> ownedApplication;
	};
}


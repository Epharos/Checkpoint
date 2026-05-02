#pragma once

#include "EditorQtCommon.hpp"
#include "QtDockHost.hpp"
#include "QtMenuBar.hpp"
#include "QtToolBar.hpp"

namespace cp::editorqt
{
	class QtApplicationWindow final : public cp::editorui::IApplicationWindow
	{
	public:
		explicit QtApplicationWindow(const cp::editorui::ApplicationWindowConfig& _config)
			: id(_config.id),
			  mainWindow(new QMainWindow()),
			  menuBar(std::make_shared<QtMenuBar>(mainWindow->menuBar())),
			  dockHost(std::make_shared<QtDockHost>(mainWindow.get()))
		{
			mainWindow->setObjectName(ToQString(_config.id));
			mainWindow->setWindowTitle(ToQString(_config.title));
			mainWindow->resize(_config.width, _config.height);
			if (_config.maximized)
			{
				mainWindow->setWindowState(Qt::WindowMaximized);
			}
		}

		[[nodiscard]] std::string_view GetId() const override { return id; }

		void SetTitle(std::string _title) override
		{
			mainWindow->setWindowTitle(ToQString(_title));
		}

		[[nodiscard]] std::string GetTitle() const override
		{
			return ToStdString(mainWindow->windowTitle());
		}

		void SetTheme(std::shared_ptr<cp::editorui::ITheme> _theme) override
		{
			activeTheme = std::move(_theme);
			if (!activeTheme)
			{
				return;
			}

			const auto windowBackground = activeTheme->GetColor(cp::editorui::ThemeColorRole::WindowBackground);
			const auto panelBackground = activeTheme->GetColor(cp::editorui::ThemeColorRole::PanelBackground);
			const auto panelBorder = activeTheme->GetColor(cp::editorui::ThemeColorRole::PanelBorder);
			const auto textPrimary = activeTheme->GetColor(cp::editorui::ThemeColorRole::TextPrimary);
			const auto accent = activeTheme->GetColor(cp::editorui::ThemeColorRole::Accent);
			const auto selection = activeTheme->GetColor(cp::editorui::ThemeColorRole::Selection);

			mainWindow->setStyleSheet(QString(
				"QMainWindow { background-color: %1; color: %2; }"
				"QDockWidget { background-color: %3; color: %2; border: 1px solid %4; }"
				"QWidget { color: %2; }"
				"QTreeView, QTreeWidget, QTableWidget { background-color: %3; border: 1px solid %4; }"
				"QLineEdit, QComboBox { background-color: %3; border: 1px solid %4; padding: 2px; }"
				"QMenuBar, QMenu, QToolBar { background-color: %1; color: %2; }"
				"QMenu::item:selected, QTreeWidget::item:selected, QTableWidget::item:selected { background-color: %5; }"
				"QPushButton, QToolButton { background-color: %6; border: 1px solid %4; padding: 4px; }")
				.arg(ColorToCss(windowBackground))
				.arg(ColorToCss(textPrimary))
				.arg(ColorToCss(panelBackground))
				.arg(ColorToCss(panelBorder))
				.arg(ColorToCss(selection))
				.arg(ColorToCss(accent)));
		}

		[[nodiscard]] std::shared_ptr<cp::editorui::IMenuBar> GetMenuBar() const override
		{
			return menuBar;
		}

		std::shared_ptr<cp::editorui::IToolBar> AddToolBar(std::string _id) override
		{
			auto* qToolbar = new QToolBar(ToQString(_id), mainWindow.get());
			mainWindow->addToolBar(qToolbar);
			auto toolbar = std::make_shared<QtToolBar>(std::move(_id), qToolbar);
			toolBars.push_back(toolbar);
			return toolbar;
		}

		[[nodiscard]] std::shared_ptr<cp::editorui::IDockHost> GetDockHost() const override
		{
			return dockHost;
		}

		void Show() override
		{
			mainWindow->show();
			if (QApplication::instance() != nullptr)
			{
				QApplication::instance()->exec();
			}
		}

		void Close() override
		{
			mainWindow->close();
		}

	private:
		std::string id;
		std::unique_ptr<QMainWindow> mainWindow;
		std::shared_ptr<QtMenuBar> menuBar;
		std::shared_ptr<QtDockHost> dockHost;
		std::vector<std::shared_ptr<QtToolBar>> toolBars;
		std::shared_ptr<cp::editorui::ITheme> activeTheme;
	};
}


#pragma once

#include "EditorQtCommon.hpp"
#include "QtDockPanel.hpp"

namespace cp::editorqt
{
	class QtDockHost final : public cp::editorui::IDockHost
	{
	public:
		explicit QtDockHost(QMainWindow* _mainWindow)
			: mainWindow(_mainWindow)
		{
		}

		std::shared_ptr<cp::editorui::IDockPanel> CreatePanel(const cp::editorui::DockPanelConfig& _config) override
		{
			auto panel = std::make_shared<QtDockPanel>(mainWindow, _config);
			panels[_config.id] = panel;
			return panel;
		}

		std::shared_ptr<cp::editorui::IDockPanel> FindPanel(std::string_view _panelId) const override
		{
			const auto it = panels.find(std::string(_panelId));
			return it == panels.end() ? nullptr : it->second;
		}

		void DockPanel(std::shared_ptr<cp::editorui::IDockPanel> _panel, const cp::editorui::DockPlacement& _placement) override
		{
			const auto panel = std::dynamic_pointer_cast<QtDockPanel>(_panel);
			if (!panel)
			{
				throw std::invalid_argument("Unsupported panel implementation for Qt dock host.");
			}

			if (_placement.area == cp::editorui::DockArea::Center)
			{
				QDockWidget* previousDockWidget = panel->GetDockWidget();
				mainWindow->removeDockWidget(previousDockWidget);
				previousDockWidget->hide();

				QWidget* centralWidget = panel->TakeAsCentralWidget();
				if (
					QWidget* currentCentral = mainWindow->takeCentralWidget();
					currentCentral != nullptr && currentCentral != centralWidget
				)
				{
					currentCentral->setParent(nullptr);
					currentCentral->hide();
				}

				mainWindow->setCentralWidget(centralWidget);
				centralWidget->show();
				return;
			}

			QDockWidget* dockWidget = panel->EnsureDockWidget();
			if (_placement.area == cp::editorui::DockArea::Floating)
			{
				dockWidget->setFloating(true);
				dockWidget->show();
				return;
			}

			mainWindow->addDockWidget(ToQtDockArea(_placement.area), dockWidget);

			if (!_placement.relativeToPanelId.empty())
			{
				const auto relative = std::dynamic_pointer_cast<QtDockPanel>(FindPanel(_placement.relativeToPanelId));
				if (relative)
				{
					if (_placement.asTab)
					{
						mainWindow->tabifyDockWidget(relative->GetDockWidget(), dockWidget);
						dockWidget->raise();
					}
					else
					{
						mainWindow->splitDockWidget(relative->GetDockWidget(), dockWidget, ToQtOrientation(_placement.area));
					}
				}
			}
		}

		void SetPanelVisible(std::string_view _panelId, const bool _visible) override
		{
			if (const auto panel = FindPanel(_panelId))
			{
				panel->SetVisible(_visible);
			}
		}

		[[nodiscard]] bool IsPanelVisible(std::string_view _panelId) const override
		{
			if (const auto panel = FindPanel(_panelId))
			{
				return panel->IsVisible();
			}

			return false;
		}

		[[nodiscard]] std::vector<std::shared_ptr<cp::editorui::IDockPanel>> GetPanels() const override
		{
			std::vector<std::shared_ptr<cp::editorui::IDockPanel>> output;
			output.reserve(panels.size());

			for (const auto& [_, panel] : panels)
			{
				output.push_back(panel);
			}

			return output;
		}

	private:
		QMainWindow* mainWindow = nullptr;
		std::unordered_map<std::string, std::shared_ptr<QtDockPanel>> panels;
	};
}


#pragma once

#include "EditorQtCommon.hpp"
#include "QtToolBar.hpp"

namespace cp::editorqt
{
	class QtDockPanel final : public cp::editorui::IDockPanel
	{
	public:
		QtDockPanel(QMainWindow* _mainWindow, cp::editorui::DockPanelConfig _config)
			: mainWindow(_mainWindow),
			  id(std::move(_config.id)),
			  title(std::move(_config.title)),
			  dockWidget(new QDockWidget(ToQString(title), mainWindow)),
			  rootContainer(new QWidget()),
			  rootLayout(new QVBoxLayout(rootContainer))
		{
			dockWidget->setObjectName(ToQString(id));
			dockWidget->setFeatures(_config.closable
				                        ? QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable
				                        : QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
			dockWidget->setVisible(_config.visible);

			rootLayout->setContentsMargins(0, 0, 0, 0);
			dockWidget->setWidget(rootContainer);
		}

		[[nodiscard]] std::string_view GetId() const override { return id; }
		[[nodiscard]] std::string_view GetTitle() const override { return title; }

		void SetTitle(std::string _title) override
		{
			title = std::move(_title);
			dockWidget->setWindowTitle(ToQString(title));
		}

		void SetVisible(const bool _visible) override
		{
			if (isCentralWidget)
			{
				rootContainer->setVisible(_visible);
				return;
			}

			dockWidget->setVisible(_visible);
		}

		[[nodiscard]] bool IsVisible() const override
		{
			if (isCentralWidget)
			{
				return rootContainer->isVisible();
			}

			return dockWidget->isVisible();
		}

		void SetContent(std::shared_ptr<cp::editorui::IWidget> _content) override
		{
			auto* access = dynamic_cast<IQtWidgetAccess*>(_content.get());
			if (access == nullptr)
			{
				throw std::invalid_argument("Unsupported widget implementation for Qt dock panel.");
			}

			QWidget* contentWidget = access->GetQWidget();
			if (currentContentWidget == contentWidget)
			{
				content = std::move(_content);
				return;
			}

			if (currentContentWidget != nullptr)
			{
				rootLayout->removeWidget(currentContentWidget);
				currentContentWidget->setParent(nullptr);
			}

			currentContentWidget = contentWidget;
			rootLayout->addWidget(currentContentWidget, 1);
			content = std::move(_content);
		}

		[[nodiscard]] std::shared_ptr<cp::editorui::IWidget> GetContent() const override
		{
			return content;
		}

		std::shared_ptr<cp::editorui::IToolBar> AddToolBar(std::string _id) override
		{
			auto* toolbarWidget = new QToolBar(rootContainer);
			toolbarWidget->setMovable(false);
			auto toolbar = std::make_shared<QtToolBar>(std::move(_id), toolbarWidget);
			rootLayout->insertWidget(static_cast<int>(toolBars.size()), toolbarWidget, 0);
			toolBars.push_back(toolbar);
			return toolbar;
		}

		[[nodiscard]] QDockWidget* GetDockWidget() const
		{
			return dockWidget.get();
		}

		[[nodiscard]] QDockWidget* EnsureDockWidget()
		{
			if (isCentralWidget)
			{
				if (mainWindow->centralWidget() == rootContainer)
				{
					mainWindow->takeCentralWidget();
				}

				dockWidget->setWidget(rootContainer);
				isCentralWidget = false;
			}

			return dockWidget.get();
		}

		[[nodiscard]] QWidget* TakeAsCentralWidget()
		{
			if (!isCentralWidget)
			{
				dockWidget->setWidget(nullptr);
				rootContainer->setParent(nullptr);
				isCentralWidget = true;
			}

			return rootContainer;
		}

	private:
		QMainWindow* mainWindow = nullptr;
		std::string id;
		std::string title;
		std::unique_ptr<QDockWidget> dockWidget;
		QWidget* rootContainer = nullptr;
		QVBoxLayout* rootLayout = nullptr;
		QWidget* currentContentWidget = nullptr;
		std::shared_ptr<cp::editorui::IWidget> content;
		std::vector<std::shared_ptr<QtToolBar>> toolBars;
		bool isCentralWidget = false;
	};
}


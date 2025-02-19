#pragma once

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qtreeview.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qfilesystemmodel.h>
#include <QtGui/qvulkanwindow.h>
#include <QtGui/qvulkaninstance.h>
#include <QtCore/qtimer.h>

#include <Core.hpp>

#include "Renderers/MinimalistRenderer.hpp"
#include "VulkanRenderer.hpp"

class MainWindow : public QMainWindow
{
	Q_OBJECT
protected:
	Context::VulkanContext vulkanContext;
	Render::Renderer* activeRenderer = nullptr;
	Core::Scene* currentScene = nullptr;

	VulkanWindow* window = nullptr;
	QVulkanInstance* instance = nullptr;

	QTreeView* fileExplorer = nullptr;
	QFileSystemModel* fileSystemModel = nullptr;

	QTreeWidget* sceneHierarchy = nullptr;

	void SetupMenuBar()
	{
		QMenuBar* menuBar = new QMenuBar;
		QMenu* fileMenu = menuBar->addMenu("File");
		QMenu* projectMenu = menuBar->addMenu("Project");
		QMenu* windowMenu = menuBar->addMenu("Window");
		QMenu* toolsMenu = menuBar->addMenu("Tools");
		QMenu* helpMenu = menuBar->addMenu("Help");

		QAction* newAction = fileMenu->addAction("New project");
		QAction* openAction = fileMenu->addAction("Open project");
		QAction* saveAction = fileMenu->addAction("Save project");
		QAction* closeAction = fileMenu->addAction("Close project");

		QAction* undoAction = fileMenu->addAction("Undo");
		QAction* redoAction = fileMenu->addAction("Redo");

		QAction* createNewSceneAction = projectMenu->addAction("Create new scene");

		QAction* openSceneHierarchyAction = windowMenu->addAction("Scene hierarchy");
		QAction* openInspectorAction = windowMenu->addAction("Inspector");
		QAction* openConsoleAction = windowMenu->addAction("Console");
		QAction* openFileExplorerAction = windowMenu->addAction("File explorer");

		connect(newAction, &QAction::triggered, [=] {
			// Create new project
			});

		connect(openAction, &QAction::triggered, [=] {
			// Open project
			});

		connect(saveAction, &QAction::triggered, [=] {
			// Save project
			});

		connect(closeAction, &QAction::triggered, [=] {
			// Close project
			});

		connect(undoAction, &QAction::triggered, [=] {
			// Undo
			});

		connect(redoAction, &QAction::triggered, [=] {
			// Redo
			});

		connect(createNewSceneAction, &QAction::triggered, [=] {
			// TODO : Save current scene
			currentScene = new Core::Scene(activeRenderer);
			window->SetScene(currentScene);
			});

		connect(openSceneHierarchyAction, &QAction::triggered, [=] {
			CreateSceneHierarchyDockWidget();
			});

		connect(openInspectorAction, &QAction::triggered, [=] {
			// Open inspector
			});

		connect(openConsoleAction, &QAction::triggered, [=] {
			// Open console
			});

		connect(openFileExplorerAction, &QAction::triggered, [=] {
			// Open file explorer
			});

		setMenuBar(menuBar);
	}

	void CreateSceneHierarchyDockWidget()
	{
		QDockWidget* dock = new QDockWidget("Scene hierarchy", this);
		if(!sceneHierarchy) sceneHierarchy = new QTreeWidget(dock);

		sceneHierarchy->setHeaderLabel("CURRENT SCENE NAME");
		dock->setWidget(sceneHierarchy);
		dock->setFloating(true);
		addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);

		/*for (const auto& entity : currentScene->GetECS().)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(sceneHierarchy);
			item->setText(0, entity->GetName().c_str());
		}*/
	}

	void CreateFileExplorerDockWidget()
	{
		QDockWidget* dock = new QDockWidget("File explorer", this);
		
	}
public:
	MainWindow(QWidget* parent = nullptr)
	{
		SetupMenuBar();

		CreateSceneHierarchyDockWidget();

		instance = new QVulkanInstance;
		instance->create();

		window = new VulkanWindow(currentScene);
		window->setVulkanInstance(instance);

		QWidget* container = QWidget::createWindowContainer(window);
		setCentralWidget(container);

		resize(1280, 720);

		QTimer::singleShot(0, this, &MainWindow::InitializeVulkanRenderer);
	}

	void InitializeVulkanRenderer()
	{
		Context::VulkanContextInfo contextInfo =
		{
			.appName = "App Example",
			.appVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
			.extensions =
			{
				.instanceExtensions =
				{},
				.instanceLayers =
				{}
			}
		};

		contextInfo.instance = instance->vkInstance();
		contextInfo.surface = QVulkanInstance::surfaceForWindow(window);
		Context::PlatformQt* platform = new Context::PlatformQt;
		platform->Initialize(window);
		contextInfo.platform = platform;

		vulkanContext.Initialize(contextInfo);

		activeRenderer = new MinimalistRenderer(&vulkanContext);
		activeRenderer->Build();
		currentScene = new Core::Scene(activeRenderer);
	}
};
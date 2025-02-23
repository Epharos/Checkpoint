#pragma once

#include "pch.hpp"

#include "Widgets/TreeEntityItem.hpp"
#include "Widgets/Inspector.hpp"

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

	ProjectData projectData;

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
			CreateInspectorDockWidget();
			});

		connect(openConsoleAction, &QAction::triggered, [=] {
			// Open console
			});

		connect(openFileExplorerAction, &QAction::triggered, [=] {
			CreateFileExplorerDockWidget();
			});

		setMenuBar(menuBar);
	}

	void CreateSceneHierarchyDockWidget()
	{
		QDockWidget* dock = new QDockWidget("Scene hierarchy", this);
		if (!sceneHierarchy)
		{
			QMenu* contextMenu = new QMenu(sceneHierarchy);
			QAction* createEntityAction = new QAction("Create entity", sceneHierarchy);
			contextMenu->addAction(createEntityAction);

			QMenu* itemContextMenu = new QMenu(sceneHierarchy);
			QAction* deleteEntityAction = new QAction("Delete entity", sceneHierarchy);
			itemContextMenu->addAction(deleteEntityAction);
			itemContextMenu->addSeparator();
			itemContextMenu->addAction(createEntityAction);

			connect(createEntityAction, &QAction::triggered, [=] {
					TreeEntityItem* item = new TreeEntityItem(currentScene->GetECS().CreateEntity(), sceneHierarchy);
					item->setFlags(item->flags() | Qt::ItemIsEditable);
					item->setSelected(true);
					sceneHierarchy->editItem(item);
				});

			connect(deleteEntityAction, &QAction::triggered, [=] {
					QList<QTreeWidgetItem*> items = sceneHierarchy->selectedItems();

					for (QTreeWidgetItem* item : items)
					{
						TreeEntityItem* entityItem = dynamic_cast<TreeEntityItem*>(item);

						if (entityItem)
						{
							currentScene->GetECS().DestroyEntity(entityItem->GetEntity());
							delete entityItem;
						}
					}
				});

			sceneHierarchy = new QTreeWidget(dock);
			sceneHierarchy->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
			sceneHierarchy->setSelectionMode(QAbstractItemView::ExtendedSelection);

			connect(sceneHierarchy, &QTreeWidget::customContextMenuRequested, [=] (QPoint pos) {
					QTreeWidgetItem* item = sceneHierarchy->itemAt(pos);

					if(item && item->isSelected()) itemContextMenu->popup(sceneHierarchy->viewport()->mapToGlobal(pos));
					else contextMenu->popup(sceneHierarchy->viewport()->mapToGlobal(pos));
				});

			connect(sceneHierarchy, &QTreeWidget::itemChanged, [=](QTreeWidgetItem* item, int column) {
				TreeEntityItem* entityItem = dynamic_cast<TreeEntityItem*>(item);

				if (entityItem)
				{
					entityItem->GetEntity().SetDisplayName(item->text(0).toStdString());
				}
				});
		}

		sceneHierarchy->setHeaderLabel(currentScene ? QString::fromStdString(currentScene->GetName()) : "No scene selected");
		dock->setWidget(sceneHierarchy);
		dock->setFloating(true);
		addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);
	}

	void CreateFileExplorerDockWidget()
	{
		QFileSystemModel* fileSystemModel = new QFileSystemModel;
		fileSystemModel->setRootPath(projectData.path + "/Resources");

		QTreeView* fileExplorer = new QTreeView;
		fileExplorer->setModel(fileSystemModel);
		fileExplorer->setRootIndex(fileSystemModel->index(projectData.path + "/Resources"));

		QDockWidget* dock = new QDockWidget("File explorer", this);
		dock->setWidget(fileExplorer);
		dock->setFloating(true);
		addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);
	}

	void CreateInspectorDockWidget()
	{
		QDockWidget* dock = new QDockWidget("Inspector", this);
		Inspector* inspector = new Inspector(currentScene, dock);
		dock->setWidget(inspector);
		dock->setFloating(true);
		addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

		connect(sceneHierarchy, &QTreeWidget::itemSelectionChanged, [=] {
			QList<QTreeWidgetItem*> items = sceneHierarchy->selectedItems();

			if (items.size() == 1)
			{
				TreeEntityItem* entityItem = dynamic_cast<TreeEntityItem*>(items[0]);

				if (entityItem)
				{
					inspector->ShowEntity(&entityItem->GetEntity());
				}
			}
			else
			{
				inspector->ShowFile("");
			}
			});
	}


public:
	MainWindow(const ProjectData& _projectData, QWidget* parent = nullptr)
	{
		SetupMenuBar();

		projectData = ProjectData(_projectData);

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
	}
};
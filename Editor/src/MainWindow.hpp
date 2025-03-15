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
	cp::VulkanContext vulkanContext;
	cp::Renderer* activeRenderer = nullptr;
	cp::Scene* currentScene = nullptr;

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
		QAction* closeAction = fileMenu->addAction("Close project");

		QAction* undoAction = fileMenu->addAction("Undo");
		QAction* redoAction = fileMenu->addAction("Redo");

		QAction* createNewSceneAction = projectMenu->addAction("Create new scene");
		QAction* saveSceneAction = projectMenu->addAction("Save scene");

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

		connect(saveSceneAction, &QAction::triggered, [=] {
			std::string path = projectData.path.toStdString() + "/Resources/Scenes/" + sceneHierarchy->headerItem()->text(0).toStdString() + ".scn";
			std::replace(path.begin(), path.end(), ' ', '_');
			cp::JsonSerializer serializer;
			currentScene->Serialize(serializer);
			serializer.Write(path);
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
			currentScene = new cp::Scene(activeRenderer);
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

	void CreateSceneHierarchyDockWidget(bool floating = true)
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

			for (const auto& entity : currentScene->GetECS().GetEntities())
			{
				TreeEntityItem* item = new TreeEntityItem(entity, sceneHierarchy);
				item->setFlags(item->flags() | Qt::ItemIsEditable);
				item->setText(0, QString::fromStdString(entity.GetDisplayName()));
			}

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
		sceneHierarchy->headerItem()->setFlags(sceneHierarchy->headerItem()->flags() | Qt::ItemIsEditable);
		dock->setWidget(sceneHierarchy);
		dock->setFloating(floating);
		addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);
	}

	void CreateFileExplorerDockWidget(bool floating = true)
	{
		QFileSystemModel* fileSystemModel = new QFileSystemModel;
		fileSystemModel->setRootPath(projectData.path + "/Resources");

		QTreeView* fileExplorer = new QTreeView;
		fileExplorer->setModel(fileSystemModel);
		fileExplorer->setRootIndex(fileSystemModel->index(projectData.path + "/Resources"));
		fileExplorer->setDragEnabled(true);
		fileExplorer->setSelectionMode(QAbstractItemView::SingleSelection);
		fileExplorer->setDragDropMode(QAbstractItemView::DragDropMode::DragOnly);

		QDockWidget* dock = new QDockWidget("File explorer", this);
		dock->setWidget(fileExplorer);
		dock->setFloating(floating);
		addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dock);

		connect(fileExplorer, &QTreeView::doubleClicked, [=](const QModelIndex& index) {
			QString path = fileSystemModel->filePath(index);
			QFileInfo fileInfo(path);

			if (fileInfo.isFile())
			{
				if (fileInfo.suffix().endsWith("scn"))
				{
					cp::JsonSerializer serializer;
					serializer.Read(path.toStdString());
					
					delete currentScene;
					currentScene = new cp::Scene(activeRenderer);
					currentScene->Deserialize(serializer);

					if(sceneHierarchy)
					{
						sceneHierarchy->headerItem()->setText(0, QString::fromStdString(currentScene->GetName()));

						for (const auto& entity : currentScene->GetECS().GetEntities())
						{
							TreeEntityItem* item = new TreeEntityItem(entity, sceneHierarchy);
							item->setFlags(item->flags() | Qt::ItemIsEditable);
							item->setText(0, QString::fromStdString(entity.GetDisplayName()));
						}
					}
				}
			}
			});
	}

	void CreateInspectorDockWidget(bool floating = true)
	{
		QDockWidget* dock = new QDockWidget("Inspector", this);
		Inspector* inspector = new Inspector(currentScene, dock);
		dock->setWidget(inspector);
		dock->setFloating(floating);
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
		Project::data = projectData;

		instance = new QVulkanInstance;
		instance->create();

		window = new VulkanWindow(currentScene);
		window->setVulkanInstance(instance);

		QWidget* container = QWidget::createWindowContainer(window);
		setCentralWidget(container);

		resize(1824, 1026);

		/*CreateSceneHierarchyDockWidget(false);
		CreateFileExplorerDockWidget(false);
		CreateInspectorDockWidget(false);*/

		QTimer::singleShot(0, this, &MainWindow::InitializeVulkanRenderer);
	}

	void InitializeVulkanRenderer()
	{
		cp::VulkanContextInfo contextInfo =
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
		cp::PlatformQt* platform = new cp::PlatformQt;
		platform->Initialize(window);
		contextInfo.platform = platform;

		vulkanContext.Initialize(contextInfo);

		cp::ResourceManager::Create(vulkanContext);

		activeRenderer = new MinimalistRenderer(&vulkanContext);
		activeRenderer->Build();
	}
};
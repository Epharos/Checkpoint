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

class MainWindow : public QMainWindow
{
	Q_OBJECT
protected:

	QTreeView* fileExplorer = nullptr;
	QFileSystemModel* fileSystemModel = nullptr;

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
		QTreeWidget* sceneHierarchy = new QTreeWidget(dock);

		sceneHierarchy->setHeaderLabel("Scene entities");
		dock->setWidget(sceneHierarchy);
		dock->setFloating(true);
		addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);

		QTreeWidgetItem* item = new QTreeWidgetItem(sceneHierarchy);
		item->setText(0, "Root");
		item->setFlags(item->flags() | Qt::ItemIsEditable);

		QTreeWidgetItem* child = new QTreeWidgetItem(item);
		child->setText(0, "Child");
		child->setFlags(child->flags() | Qt::ItemIsEditable);

		sceneHierarchy->addTopLevelItem(item);
		sceneHierarchy->addTopLevelItem(child);
	}
	void CreateFileExplorerDockWidget()
	{
		QDockWidget* dock = new QDockWidget("File explorer", this);
		
	}
public:
	MainWindow(QWidget* parent = nullptr)
	{
		QVulkanInstance* instance = new QVulkanInstance;
		instance->create();

		QVulkanWindow* window = new QVulkanWindow;
		window->setVulkanInstance(instance);

		QWidget* container = QWidget::createWindowContainer(window);
		setCentralWidget(container);

		SetupMenuBar();

		setWindowTitle("Checkpoint Editor");
		resize(1280, 720);
	}
};
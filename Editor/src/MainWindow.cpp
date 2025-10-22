#include "MainWindow.hpp"

import PluginLoader;
import EditorUI;

MainWindow::MainWindow(const ProjectData& _projectData, QWidget* parent)
{
	projectData = ProjectData(_projectData);
	Project::data = projectData;

	cp::PluginContext ctx;
	ctx.version = ctx.MakeVersion(1, 0, 0);

	cp::PluginLoader pluginLoader{ ctx };
	size_t loadedPlugins = pluginLoader.ScanPlugins();

	LOG_INFO(MF("Loaded ", loadedPlugins, " plugin", loadedPlugins > 1 ? "s!" : "!"));

	SetupMenuBar();

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
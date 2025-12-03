#include "EditorWindow.hpp"

#include "ECSWrapper.hpp"
#include "EditorUI/QtWidgets/SceneHierarchy.hpp"
#include "Renderers/EditorRenderer.hpp"
#include <QObject>

cp::EditorWindow::EditorWindow(cp::Project _project)
{
	cp::QtEditorUIFactory* factory = new cp::QtEditorUIFactory();

	window = factory->CreateWindow();
	window->Show();

	auto dockSceneHierarchy = factory->CreateDockableWindow(window.get());
	dockSceneHierarchy->SetTitle("Scene Hierarchy");
	dockSceneHierarchy->Show();

	auto sceneHierarchy = factory->CreateSceneHierarchy();
	auto containerSceneHierarchy = factory->CreateContainer().release();
	containerSceneHierarchy->AddChild(sceneHierarchy.get());
	dockSceneHierarchy->SetContainer(containerSceneHierarchy);

	QObject::connect((cp::SceneHierarchy*)sceneHierarchy->NativeHandle(), &cp::SceneHierarchy::SceneUpdated, [&](const cp::SceneAsset* _scene) {
		if (_scene) {
			LOG_INFO(MF("Scene updated: ", _scene->name));
		}

		dockSceneHierarchy->SetTitle("Scene Hierarchy: " + _scene->name);
		});

	auto dockViewport = factory->CreateDockableWindow();
	dockViewport->SetTitle("Viewport");
	dockViewport->DockTo(dockSceneHierarchy.get(), cp::DockArea::Right);
	dockViewport->Show();

	cp::SceneAsset* newScene = new cp::SceneAsset();
	newScene->renderer = new cp::EditorRenderer(&cp::CheckpointEditor::VulkanCtx);
	newScene->name = "Newly created scene";

	auto viewport = factory->CreateViewport(newScene);
	auto containerViewport = factory->CreateContainer().release();
	containerViewport->AddChild(viewport.get());
	dockViewport->SetContainer(containerViewport);

	auto dockInspector = factory->CreateDockableWindow(nullptr);
	dockInspector->SetTitle("Inspector");
	dockInspector->DockTo(dockViewport.get(), cp::DockArea::Right);
	dockInspector->Show();

	auto inspector = factory->CreateInspector();
	auto containerInspector = factory->CreateContainer().release();
	containerInspector->AddChild(inspector.get());
	dockInspector->SetContainer(containerInspector);

	QObject::connect((cp::SceneHierarchy*)sceneHierarchy->NativeHandle(), &cp::SceneHierarchy::EntitySelected, [&](cp::EntityAsset* _entity) {
		if (_entity) {
			LOG_INFO(MF("Entity selected: ", _entity->name));
		}

		inspector->ShowEntity(_entity);
		});

	auto dockAssetBrowser = factory->CreateDockableWindow(nullptr);
	dockAssetBrowser->SetTitle("Asset Browser");
	dockAssetBrowser->DockTo(dockSceneHierarchy.get(), cp::DockArea::Bottom);
	dockAssetBrowser->Show();

	auto assetBrowser = factory->CreateAssetBrowser(cp::CheckpointEditor::CurrentProject.GetResourcePath());
	assetBrowser->LinkToInspector(inspector.get());
	auto containerAssetBrowser = factory->CreateContainer().release();
	containerAssetBrowser->AddChild(assetBrowser.get());
	dockAssetBrowser->SetContainer(containerAssetBrowser);

	dockedWindows.push_back(std::move(dockSceneHierarchy));
	dockedWindows.push_back(std::move(dockViewport));
	dockedWindows.push_back(std::move(dockInspector));
	dockedWindows.push_back(std::move(dockAssetBrowser));
}

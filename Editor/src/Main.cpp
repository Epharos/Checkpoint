#include "pch.hpp"

#include <QtCore/qobject.h>
#include <QtGui/qfontdatabase.h>
#include "MainWindow.hpp"
#include "Launcher.hpp"

#include "Components/Transform.hpp"
#include "Components/MeshRenderer.hpp"

#include "ECSWrapper.hpp"

#include "EditorUI/QtWidgets/SceneHierarchy.hpp"

import EditorUI;

QString LoadStyleSheet(const QString& path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
	{
		std::cerr << "Failed to open file: " << path.toStdString() << std::endl;
		return "";
	}

	QString styleSheet = file.readAll();
	file.close();

	return styleSheet;
}

int main(int argc, char* args[])
{
	QApplication app(argc, args);

	cp::ComponentRegistry::GetInstance().Register<Transform, TransformWidget, TransformSerializer>("Transform");
	cp::ComponentRegistry::GetInstance().Register<MeshRenderer, MeshRendererWidget, MeshRendererSerializer>("Mesh Renderer");

	app.setStyleSheet(LoadStyleSheet("Editor_Resources/Stylesheet.qss"));

	int fontID = QFontDatabase::addApplicationFont("Editor_Resources/Montserrat.ttf");

	fontID = QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Medium.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-MediumItalic.ttf");
	fontID = QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Book.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-BookItalic.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Bold.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-BoldItalic.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Black.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-BlackItalic.ttf");

	QStringList families = QFontDatabase::applicationFontFamilies(fontID);
	qDebug() << families;

	if (fontID != -1)
	{
		QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontID);

		if (!fontFamilies.isEmpty())
		{
			QFont font(fontFamilies.first(), 10);
			font.setStyleStrategy(QFont::PreferQuality);
			font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
			app.setFont(font);
		}
		else
		{
			std::cerr << "Failed to load font." << std::endl;
		}
	}
	else
	{
		std::cerr << "Failed to load font." << std::endl;
	}

	Launcher launcher;
	launcher.show();

	cp::QtEditorUIFactory factory;
	auto win = factory.CreateWindow();
	win->Show();

	auto dock = factory.CreateDockableWindow(win.get());
	dock->SetTitle("Scene Hierarchy");
	dock->Show();

	auto sh = factory.CreateSceneHierarchy();
	auto c = factory.CreateContainer().release();
	c->AddChild(sh.get());
	dock->SetContainer(c);

	QObject::connect((cp::SceneHierarchy*)sh->NativeHandle(), &cp::SceneHierarchy::SceneUpdated, [&](const cp::SceneAsset* _scene) {
		if (_scene) {
			LOG_INFO(MF("Scene updated: ", _scene->name));
		}

		dock->SetTitle("Scene Hierarchy: " + _scene->name);
		});

	cp::SceneAsset* scene = new cp::SceneAsset();
	scene->name = "Coucou";

	cp::EntityAsset* entity = new cp::EntityAsset();
	entity->name = "Coucou toi";
	entity->components.push_back(new Transform());
	entity->components.push_back(new MeshRenderer());

	scene->entities.push_back(entity);
	entity = new cp::EntityAsset();
	entity->name = "Je suis une autre entite";
	entity->components.push_back(new Transform());
	scene->entities.push_back(entity);
	entity = new cp::EntityAsset();
	entity->name = "Kappa";
	entity->components.push_back(new Transform());
	scene->entities.push_back(entity);
	entity = new cp::EntityAsset();
	entity->name = "EFT 15.11";
	entity->components.push_back(new MeshRenderer());
	scene->entities.push_back(entity);

	sh->UpdateScene(scene);
	dock->MatchSizeToContent();

	auto dockInspector = factory.CreateDockableWindow(nullptr);
	dockInspector->SetTitle("Inspector");
	dockInspector->DockTo(dock.get(), cp::DockArea::Left);
	dockInspector->Show();

	auto inspector = factory.CreateInspector();
	auto containerInspector = factory.CreateContainer().release();
	containerInspector->AddChild(inspector.get());
	dockInspector->SetContainer(containerInspector);

	QObject::connect((cp::SceneHierarchy*)sh->NativeHandle(), &cp::SceneHierarchy::EntitySelected, [&](cp::EntityAsset* _entity) {
		if (_entity) {
			LOG_INFO(MF("Entity selected: ", _entity->name));
		}

		inspector->ShowEntity(_entity);
	});

	return app.exec();
}
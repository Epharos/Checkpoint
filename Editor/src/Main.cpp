#include "pch.hpp"

#include <QtGui/qfontdatabase.h>
#include "MainWindow.hpp"
#include "Launcher.hpp"

#include "Components/Transform.hpp"
#include "Components/MeshRenderer.hpp"

#include "ECSWrapper.hpp"

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
	dock->Show();

	auto sh = factory.CreateSceneHierarchy();
	auto c = factory.CreateContainer();
	c->AddChild(sh.get());
	dock->SetContainer(c.get());

	cp::SceneAsset* scene = new cp::SceneAsset();
	scene->name = "Coucou";

	cp::EntityAsset entity;
	entity.name = "Coucou toi";
	scene->entities.push_back(entity);
	entity.name = "Je suis une autre entite";
	scene->entities.push_back(entity);
	entity.name = "Kappa";
	scene->entities.push_back(entity);
	entity.name = "EFT 15.11";
	scene->entities.push_back(entity);

	sh->UpdateScene(scene);

	return app.exec();
}
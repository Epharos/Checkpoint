#include "pch.hpp"

#include <QtGui/qfontdatabase.h>
#include "MainWindow.hpp"
#include "Launcher.hpp"

#include "Components/Transform.hpp"
#include "Components/MeshRenderer.hpp"

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

	return app.exec();
}
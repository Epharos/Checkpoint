#include <iostream>
#include <QtGui/qfontdatabase.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qwidget.h>
#include "MainWindow.hpp"
#include "Launcher.hpp"

int main(int argc, char* args[])
{
	QApplication app(argc, args);

	int fontID = QFontDatabase::addApplicationFont("Editor_Resources/Oswald.ttf");

	if (fontID != -1)
	{
		QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontID);

		if (!fontFamilies.isEmpty())
		{
			QFont font(fontFamilies.first(), 10);
			font.setWeight(QFont::Weight::Normal);
			font.setStyleStrategy(QFont::PreferQuality);
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

	/*MainWindow mainWindow;
	mainWindow.show();*/

	return app.exec();
}
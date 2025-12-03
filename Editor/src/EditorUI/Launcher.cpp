#include "Launcher.hpp"

cp::Launcher::Launcher()
{
	cp::QtEditorUIFactory* factory = new cp::QtEditorUIFactory();

	window = factory->CreateWindow();
	window->Show();

	auto globalContainer = factory->CreateContainer();
	globalContainer->SetVertical();

	auto columnContainer = factory->CreateContainer();
	columnContainer->SetHorizontal();

	auto recentProjectLabel = factory->CreateLabel("Recent Projects");
	globalContainer->AddChild(recentProjectLabel.release());

	auto projectList = factory->CreateProjectList();
	columnContainer->AddChild(projectList.release());

	auto overviewContainer = factory->CreateContainer();
	overviewContainer->SetVertical();

	auto overviewLabel = factory->CreateLabel("Overview");
	overviewContainer->AddChild(overviewLabel.release());

	columnContainer->AddChild(overviewContainer.release());

	globalContainer->AddChild(columnContainer.release());
	window->SetContainer(globalContainer.release());
}
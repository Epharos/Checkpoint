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

	//auto projectOverview = new IProjectOverview(factory);
	auto projectList = factory->CreateProjectList();

	//auto OnProjectFocused = [projectOverview](const std::string& _path) -> void {
	//	cp::Project proj = cp::CheckpointEditor::LoadProjectData(_path);
	//	projectOverview->SetProject(proj);
	//};

	auto OnProjectOpened = [this](const std::string& _path) -> void {
		cp::CheckpointEditor::LoadProject(_path);
		window->Close();
	};

	//projectList->AddProjectFocusedListener(OnProjectFocused);
	projectList->AddProjectOpenedListener(OnProjectOpened);

	columnContainer->AddChild(projectList.release());

	//auto overviewContainer = factory->CreateContainer();
	//overviewContainer->SetVertical();
	//overviewContainer->AddChild(projectOverview);

	//columnContainer->AddChild(overviewContainer.release());

	globalContainer->AddChild(columnContainer.release());
	window->SetContainer(globalContainer.release());

	delete factory;
}
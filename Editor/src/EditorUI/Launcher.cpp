#include "Launcher.hpp"

#include "EditorUI/EditorWindow.hpp"

cp::Launcher::Launcher()
{
	cp::QtEditorUIFactory* factory = new cp::QtEditorUIFactory();

	window = factory->CreateWindow();
	window->SetMinSize(800, 600);
	window->SetSize(800, 600);
	window->SetTitle("Checkpoint Launcher");
	window->Show();

	auto globalContainer = factory->CreateContainer(cp::ContainerOrientation::Vertical);
	auto topContainer = factory->CreateContainer(cp::ContainerOrientation::Horizontal);
	globalContainer->SetMargins(8, 8, 8, 8);
	globalContainer->SetSpacing(12);

	auto recentProjectLabel = factory->CreateLabel("Projects");
	recentProjectLabel->SetBold(true);
	recentProjectLabel->SetTextSize(18);
	topContainer->AddChild(recentProjectLabel.release());

	auto addProjectButton = factory->CreateFlatButton("+ Add Project");
	addProjectButton->SetTextSize(11);
	addProjectButton->SetSize(120, 24);
	topContainer->AddChild(addProjectButton.release());

	auto projectList = factory->CreateProjectList();

	auto OnProjectOpened = [this](const std::string& _path) -> void {
		cp::CheckpointEditor::LoadProject(_path);
		new cp::EditorWindow(cp::CheckpointEditor::CurrentProject);
		window->Close();
	};

	projectList->AddProjectOpenedListener(OnProjectOpened);

	globalContainer->AddChild(topContainer.release());
	globalContainer->AddChild(projectList.release());
	window->SetContainer(globalContainer.release());

	delete factory;
}
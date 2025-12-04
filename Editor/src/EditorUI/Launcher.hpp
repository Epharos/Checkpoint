#pragma once

#include "../pch.hpp"

#include "../CheckpointEditor.hpp"

import EditorUI;

namespace cp
{
	class IProjectOverview : public IWidget 
	{
	protected:
		std::unique_ptr<ILabel> projectNameLabel;
		std::unique_ptr<ILabel> projectPathLabel;

		std::unique_ptr<ILabel> projectCreatedLabel;
		std::unique_ptr<ILabel> projectLastOpenedLabel;

		std::unique_ptr<ILabel> projectVersionLabel;

		std::unique_ptr<ILabel> titleLabel;
		std::unique_ptr<ILabel> noSelectLabel;

		std::unique_ptr<IContainer> mainContainer;

	public:
		IProjectOverview(IEditorUIFactory* factory)
		{
			mainContainer = factory->CreateContainer();
			titleLabel = factory->CreateLabel("Project Overview");
			noSelectLabel = factory->CreateLabel("No project selected.");

			projectNameLabel = factory->CreateLabel();
			projectNameLabel->SetVisible(false);
			projectPathLabel = factory->CreateLabel();
			projectPathLabel->SetVisible(false);
			projectCreatedLabel = factory->CreateLabel();
			projectCreatedLabel->SetVisible(false);
			projectLastOpenedLabel = factory->CreateLabel();
			projectLastOpenedLabel->SetVisible(false);
			projectVersionLabel = factory->CreateLabel();
			projectVersionLabel->SetVisible(false);

			mainContainer->AddChild(titleLabel.get());
			mainContainer->AddChild(noSelectLabel.get());
			mainContainer->AddChild(projectNameLabel.get());
			mainContainer->AddChild(projectPathLabel.get());
			mainContainer->AddChild(projectCreatedLabel.get());
			mainContainer->AddChild(projectLastOpenedLabel.get());
			mainContainer->AddChild(projectVersionLabel.get());
		}

		void SetProject(const cp::Project& project)
		{
			projectNameLabel->SetText(project.name);
			projectPathLabel->SetText(project.path);
			projectCreatedLabel->SetText("Created: " + project.FormatCreationDate());
			projectLastOpenedLabel->SetText("Last Opened: " + project.FormatLastOpened());
			projectVersionLabel->SetText("Version: " + project.engineVersion.ToString());

			noSelectLabel->SetVisible(false);
			projectNameLabel->SetVisible(true);
			projectPathLabel->SetVisible(true);
			projectCreatedLabel->SetVisible(true);
			projectLastOpenedLabel->SetVisible(true);
			projectVersionLabel->SetVisible(true);
		}

		void SetVisible(bool visible) noexcept 
		{
			mainContainer->SetVisible(visible);
		}

		bool IsVisible() const noexcept 
		{
			return mainContainer->IsVisible();
		}

		void SetEnabled(bool enabled) noexcept
		{
			mainContainer->SetEnabled(enabled);
		}

		bool IsEnabled() const noexcept
		{
			return mainContainer->IsEnabled();
		}

		void* NativeHandle() const noexcept override
		{
			return mainContainer->NativeHandle();
		}
	};

	class Launcher
	{
	protected:
		std::unique_ptr<IWindow> window;
	public:
		Launcher();
	};
};
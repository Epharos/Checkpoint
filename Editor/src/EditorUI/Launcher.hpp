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

		std::unique_ptr<ILabel> noSelectLabel;

		std::unique_ptr<IContainer> mainContainer;

	public:
		virtual void SetProject(const cp::Project& project) = 0;

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

		void* NativeHandle() const noexcept = 0;
	};

	class Launcher
	{
	protected:
		std::unique_ptr<IWindow> window;
	public:
		Launcher();
	};
};
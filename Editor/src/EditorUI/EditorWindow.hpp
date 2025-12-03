#pragma once

#include "../pch.hpp"

#include "../CheckpointEditor.hpp"

import EditorUI;

namespace cp
{
	class EditorWindow
	{
	protected:
		std::unique_ptr<IWindow> window;

		std::vector<std::unique_ptr<IDockableWindow>> dockedWindows;

	public:
		EditorWindow(cp::Project _project);
	};
};
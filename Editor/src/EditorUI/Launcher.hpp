#pragma once

#include "../pch.hpp"

#include "../CheckpointEditor.hpp"

import EditorUI;

namespace cp
{
	class Launcher
	{
	protected:
		std::unique_ptr<IWindow> window;
	public:
		Launcher();
	};
};
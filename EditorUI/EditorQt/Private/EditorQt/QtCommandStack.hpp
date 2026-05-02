#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtCommandStack final : public cp::editorui::ICommandStack
	{
	public:
		void Push(std::unique_ptr<cp::editorui::ICommand> _command) override
		{
			if (!_command)
			{
				return;
			}

			if (cursor < commands.size())
			{
				commands.erase(commands.begin() + static_cast<long long>(cursor), commands.end());
			}

			_command->Execute();
			commands.push_back(std::move(_command));
			cursor = commands.size();
		}

		[[nodiscard]] bool CanUndo() const override
		{
			return cursor > 0;
		}

		[[nodiscard]] bool CanRedo() const override
		{
			return cursor < commands.size();
		}

		void Undo() override
		{
			if (!CanUndo())
			{
				return;
			}

			--cursor;
			commands[cursor]->Undo();
		}

		void Redo() override
		{
			if (!CanRedo())
			{
				return;
			}

			commands[cursor]->Execute();
			++cursor;
		}

		void Clear() override
		{
			commands.clear();
			cursor = 0;
		}

	private:
		std::vector<std::unique_ptr<cp::editorui::ICommand>> commands;
		size_t cursor = 0;
	};
}


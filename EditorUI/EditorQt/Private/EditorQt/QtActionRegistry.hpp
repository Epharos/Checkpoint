#pragma once

#include "EditorQtCommon.hpp"
#include "QtAction.hpp"

namespace cp::editorqt
{
	class QtActionRegistry final : public cp::editorui::IActionRegistry
	{
	public:
		std::shared_ptr<cp::editorui::IAction> CreateAction(const cp::editorui::ActionDescriptor& _descriptor) override
		{
			auto action = std::make_shared<QtAction>(_descriptor);
			actions[_descriptor.id] = action;
			return action;
		}

		std::shared_ptr<cp::editorui::IAction> FindAction(std::string_view _actionId) const override
		{
			const auto iterator = actions.find(std::string(_actionId));
			return iterator == actions.end() ? nullptr : iterator->second;
		}

		void BindToMenu(cp::editorui::IMenu& _menu, const std::string_view _actionId) override
		{
			const auto action = FindAction(_actionId);
			if (!action)
			{
				return;
			}
			_menu.AddAction(action);
		}

		void BindToToolBar(cp::editorui::IToolBar& _toolbar, const std::string_view _actionId) override
		{
			const auto action = FindAction(_actionId);
			if (!action)
			{
				return;
			}
			_toolbar.AddAction(action);
		}

	private:
		std::unordered_map<std::string, std::shared_ptr<cp::editorui::IAction>> actions;
	};
}


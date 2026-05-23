#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtContextMenu final : public cp::editorui::IContextMenu
	{
	public:
		explicit QtContextMenu(const std::string& _id)
			: menu(new QMenu(ToQString(_id)))
		{
		}

		void AddAction(std::shared_ptr<cp::editorui::IAction> _action) override
		{
			if (const auto* access = dynamic_cast<IQtActionAccess*>(_action.get()); access != nullptr)
			{
				menu->addAction(access->GetQAction());
			}
		}

		void AddAction(std::string _label, std::function<void()> _callback) override
		{
			QAction* action = new QAction(ToQString(_label), menu.get());
			QObject::connect(action, &QAction::triggered, [callback = std::move(_callback)]() { callback(); });
			menu->addAction(action);
		}

		void AddSeparator() override
		{
			menu->addSeparator();
		}

		void Exec(const QPoint& _globalPosition) const
		{
			menu->exec(_globalPosition);
		}

	private:
		std::unique_ptr<QMenu> menu;
	};
}


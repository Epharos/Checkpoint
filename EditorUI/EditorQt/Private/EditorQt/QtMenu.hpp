#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtMenu final : public cp::editorui::IMenu
	{
	public:
		QtMenu(std::string _id, std::string _title, QMenu* _menu)
			: id(std::move(_id)),
			  title(std::move(_title)),
			  menu(_menu)
		{
		}

		[[nodiscard]] std::string_view GetId() const override { return id; }
		[[nodiscard]] std::string_view GetTitle() const override { return title; }

		void AddAction(std::shared_ptr<cp::editorui::IAction> _action) override
		{
			if (const auto* access = dynamic_cast<IQtActionAccess*>(_action.get()); access != nullptr)
			{
				menu->addAction(access->GetQAction());
			}
		}

		std::shared_ptr<cp::editorui::IMenu> AddSubMenu(std::string _id, std::string _title) override
		{
			QMenu* subMenu = menu->addMenu(ToQString(_title));
			auto abstraction = std::make_shared<QtMenu>(std::move(_id), std::move(_title), subMenu);
			subMenus.push_back(abstraction);
			return abstraction;
		}

		void AddSeparator() override
		{
			menu->addSeparator();
		}

	private:
		std::string id;
		std::string title;
		QMenu* menu = nullptr;
		std::vector<std::shared_ptr<QtMenu>> subMenus;
	};
}


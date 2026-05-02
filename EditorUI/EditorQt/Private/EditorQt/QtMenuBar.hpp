#pragma once

#include "EditorQtCommon.hpp"
#include "QtMenu.hpp"

namespace cp::editorqt
{
	class QtMenuBar final : public cp::editorui::IMenuBar
	{
	public:
		explicit QtMenuBar(QMenuBar* _menuBar)
			: menuBar(_menuBar)
		{
		}

		std::shared_ptr<cp::editorui::IMenu> AddMenu(std::string _id, std::string _title) override
		{
			const auto key = _id;
			QMenu* menu = menuBar->addMenu(ToQString(_title));
			auto abstraction = std::make_shared<QtMenu>(std::move(_id), std::move(_title), menu);
			menus[key] = abstraction;
			return abstraction;
		}

		std::shared_ptr<cp::editorui::IMenu> FindMenu(std::string_view _id) const override
		{
			const auto iterator = menus.find(std::string(_id));
			return iterator == menus.end() ? nullptr : iterator->second;
		}

	private:
		QMenuBar* menuBar = nullptr;
		std::unordered_map<std::string, std::shared_ptr<cp::editorui::IMenu>> menus;
	};
}


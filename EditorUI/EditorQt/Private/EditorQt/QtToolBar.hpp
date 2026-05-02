#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtToolBar final : public cp::editorui::IToolBar
	{
	public:
		QtToolBar(std::string _id, QToolBar* _toolBar)
			: id(std::move(_id)),
			  toolBar(_toolBar)
		{
			toolBar->setObjectName(ToQString(id));
		}

		[[nodiscard]] std::string_view GetId() const override
		{
			return id;
		}

		void AddAction(std::shared_ptr<cp::editorui::IAction> _action) override
		{
			if (const auto* access = dynamic_cast<IQtActionAccess*>(_action.get()); access != nullptr)
			{
				toolBar->addAction(access->GetQAction());
			}
		}

		void AddSeparator() override
		{
			toolBar->addSeparator();
		}

		void SetVisible(const bool _visible) override
		{
			toolBar->setVisible(_visible);
		}

		[[nodiscard]] bool IsVisible() const override
		{
			return toolBar->isVisible();
		}

	private:
		std::string id;
		QToolBar* toolBar = nullptr;
	};
}


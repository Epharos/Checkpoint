#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtSearchBox final : public cp::editorui::ISearchBox, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtSearchBox(std::string _id)
			: QtWidgetBase(std::move(_id)),
			  lineEdit(new QLineEdit())
		{
			lineEdit->setObjectName(ToQString(GetId()));

			QObject::connect(lineEdit.get(), &QLineEdit::textChanged, [this](const QString& _text)
			{
				if (textChangedHandler)
				{
					textChangedHandler(ToStdString(_text));
				}
			});
		}

		[[nodiscard]] std::string_view GetId() const override
		{
			return QtWidgetBase::GetId();
		}

		void SetVisible(const bool _visible) override
		{
			QtWidgetBase::SetVisible(_visible);
		}

		[[nodiscard]] bool IsVisible() const override
		{
			return QtWidgetBase::IsVisible();
		}

		void SetEnabled(const bool _enabled) override
		{
			QtWidgetBase::SetEnabled(_enabled);
		}

		[[nodiscard]] bool IsEnabled() const override
		{
			return QtWidgetBase::IsEnabled();
		}

		void SetPlaceholder(std::string _placeholder) override
		{
			lineEdit->setPlaceholderText(ToQString(_placeholder));
		}

		void SetText(std::string _text) override
		{
			lineEdit->setText(ToQString(_text));
		}

		[[nodiscard]] std::string GetText() const override
		{
			return ToStdString(lineEdit->text());
		}

		void SetTextChangedHandler(TextChangedHandler _handler) override
		{
			textChangedHandler = std::move(_handler);
		}

		[[nodiscard]] QWidget* GetQWidget() const override
		{
			return lineEdit.get();
		}

	private:
		std::unique_ptr<QLineEdit> lineEdit;
		TextChangedHandler textChangedHandler;
	};
}


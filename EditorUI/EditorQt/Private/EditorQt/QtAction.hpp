#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtAction final : public cp::editorui::IAction, public IQtActionAccess
	{
	public:
		explicit QtAction(const cp::editorui::ActionDescriptor& _descriptor)
			: descriptor(_descriptor),
			  action(new QAction(ToQString(_descriptor.text)))
		{
			action->setObjectName(ToQString(_descriptor.id));
			action->setToolTip(ToQString(_descriptor.tooltip));
			action->setVisible(true);
			action->setCheckable(_descriptor.checkable);
			action->setChecked(_descriptor.checked);
			if (_descriptor.shortcut.has_value())
			{
				action->setShortcut(QKeySequence(ToQString(_descriptor.shortcut->chord)));
			}

			QObject::connect(action.get(), &QAction::triggered, [this]()
			{
				if (triggeredHandler)
				{
					triggeredHandler();
				}
			});
		}

		[[nodiscard]] QAction* GetQAction() const override
		{
			return action.get();
		}

		[[nodiscard]] std::string_view GetId() const override
		{
			return descriptor.id;
		}

		void SetText(std::string _text) override
		{
			descriptor.text = std::move(_text);
			action->setText(ToQString(descriptor.text));
		}

		[[nodiscard]] std::string_view GetText() const override
		{
			return descriptor.text;
		}

		void SetTooltip(std::string _tooltip) override
		{
			descriptor.tooltip = std::move(_tooltip);
			action->setToolTip(ToQString(descriptor.tooltip));
		}

		void SetEnabled(const bool _enabled) override
		{
			action->setEnabled(_enabled);
		}

		[[nodiscard]] bool IsEnabled() const override
		{
			return action->isEnabled();
		}

		void SetVisible(const bool _visible) override
		{
			action->setVisible(_visible);
		}

		[[nodiscard]] bool IsVisible() const override
		{
			return action->isVisible();
		}

		void SetCheckable(const bool _checkable) override
		{
			action->setCheckable(_checkable);
		}

		[[nodiscard]] bool IsCheckable() const override
		{
			return action->isCheckable();
		}

		void SetChecked(const bool _checked) override
		{
			action->setChecked(_checked);
		}

		[[nodiscard]] bool IsChecked() const override
		{
			return action->isChecked();
		}

		void SetTriggeredHandler(TriggeredHandler _handler) override
		{
			triggeredHandler = std::move(_handler);
		}

		void Trigger() override
		{
			action->trigger();
		}

		void SetShortcut(std::optional<std::string> _chord) override
		{
			if (_chord.has_value())
			{
				descriptor.shortcut = cp::editorui::Shortcut{ *_chord };
				action->setShortcut(QKeySequence(ToQString(*_chord)));
				return;
			}

			descriptor.shortcut = std::nullopt;
			action->setShortcut(QKeySequence());
		}

		[[nodiscard]] std::optional<std::string> GetShortcut() const override
		{
			if (descriptor.shortcut.has_value())
			{
				return descriptor.shortcut->chord;
			}

			return std::nullopt;
		}

	private:
		cp::editorui::ActionDescriptor descriptor;
		std::unique_ptr<QAction> action;
		TriggeredHandler triggeredHandler;
	};
}


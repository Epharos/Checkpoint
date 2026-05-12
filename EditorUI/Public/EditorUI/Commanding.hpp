#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <EditorUI/Types.hpp>

namespace cp::editorui
{
	class IAction;
	class IMenu;
	class IToolBar;

	struct Shortcut
	{
		std::string chord;
		bool editorGlobal = true;
	};

	struct ActionDescriptor
	{
		std::string id;
		std::string text;
		std::string tooltip;
		std::string icon;
		std::optional<Shortcut> shortcut;
		bool checkable = false;
		bool checked = false;
	};

	class IAction
	{
	public:
		using TriggeredHandler = std::function<void()>;

		virtual ~IAction() = default;

		virtual std::string_view GetId() const = 0;
		virtual void SetText(std::string _text) = 0;
		virtual std::string_view GetText() const = 0;
		virtual void SetTooltip(std::string _tooltip) = 0;
		virtual void SetEnabled(bool _enabled) = 0;
		virtual bool IsEnabled() const = 0;
		virtual void SetVisible(bool _visible) = 0;
		virtual bool IsVisible() const = 0;
		virtual void SetCheckable(bool _checkable) = 0;
		virtual bool IsCheckable() const = 0;
		virtual void SetChecked(bool _checked) = 0;
		virtual bool IsChecked() const = 0;
		virtual void SetTriggeredHandler(TriggeredHandler _handler) = 0;
		virtual void Trigger() = 0;

		virtual void SetShortcut(std::optional<std::string> _chord) = 0;
		virtual std::optional<std::string> GetShortcut() const = 0;
	};

	class ICommand
	{
	public:
		virtual ~ICommand() = default;

		virtual std::string_view GetLabel() const = 0;
		virtual void Execute() = 0;
		virtual void Undo() = 0;
	};

	class ICommandStack
	{
	public:
		virtual ~ICommandStack() = default;

		virtual void Push(std::unique_ptr<ICommand> _command) = 0;
		virtual bool CanUndo() const = 0;
		virtual bool CanRedo() const = 0;
		virtual void Undo() = 0;
		virtual void Redo() = 0;
		virtual void Clear() = 0;
	};

	class IActionRegistry
	{
	public:
		virtual ~IActionRegistry() = default;

		virtual std::shared_ptr<IAction> CreateAction(const ActionDescriptor& _descriptor) = 0;
		virtual std::shared_ptr<IAction> FindAction(std::string_view _actionId) const = 0;
		virtual void BindToMenu(IMenu& _menu, std::string_view _actionId) = 0;
		virtual void BindToToolBar(IToolBar& _toolbar, std::string_view _actionId) = 0;
	};
}

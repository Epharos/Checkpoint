#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <EditorUI/Commanding.hpp>
#include <EditorUI/Types.hpp>
#include <EditorUI/Widgets.hpp>

namespace cp::editorui
{
	class ITheme
	{
	public:
		virtual ~ITheme() = default;

		virtual std::string_view GetName() const = 0;
		virtual void SetColor(ThemeColorRole _role, RGBA8 _color) = 0;
		virtual RGBA8 GetColor(ThemeColorRole _role) const = 0;
	};

	class IMenu
	{
	public:
		virtual ~IMenu() = default;

		virtual std::string_view GetId() const = 0;
		virtual std::string_view GetTitle() const = 0;
		virtual void AddAction(std::shared_ptr<IAction> _action) = 0;
		virtual std::shared_ptr<IMenu> AddSubMenu(std::string _id, std::string _title) = 0;
		virtual void AddSeparator() = 0;
	};

	class IMenuBar
	{
	public:
		virtual ~IMenuBar() = default;

		virtual std::shared_ptr<IMenu> AddMenu(std::string _id, std::string _title) = 0;
		virtual std::shared_ptr<IMenu> FindMenu(std::string_view _id) const = 0;
	};

	class IToolBar
	{
	public:
		virtual ~IToolBar() = default;

		virtual std::string_view GetId() const = 0;
		virtual void AddAction(std::shared_ptr<IAction> _action) = 0;
		virtual void AddSeparator() = 0;
		virtual void SetVisible(bool _visible) = 0;
		virtual bool IsVisible() const = 0;
	};

	class IDockPanel
	{
	public:
		virtual ~IDockPanel() = default;

		virtual std::string_view GetId() const = 0;
		virtual std::string_view GetTitle() const = 0;
		virtual void SetTitle(std::string _title) = 0;
		virtual void SetVisible(bool _visible) = 0;
		virtual bool IsVisible() const = 0;
		virtual void SetContent(std::shared_ptr<IWidget> _content) = 0;
		virtual std::shared_ptr<IWidget> GetContent() const = 0;
		virtual std::shared_ptr<IToolBar> AddToolBar(std::string _id) = 0;
	};

	class IDockHost
	{
	public:
		virtual ~IDockHost() = default;

		virtual std::shared_ptr<IDockPanel> CreatePanel(const DockPanelConfig& _config) = 0;
		virtual std::shared_ptr<IDockPanel> FindPanel(std::string_view _panelId) const = 0;
		virtual void DockPanel(std::shared_ptr<IDockPanel> _panel, const DockPlacement& _placement) = 0;
		virtual void SetPanelVisible(std::string_view _panelId, bool _visible) = 0;
		virtual bool IsPanelVisible(std::string_view _panelId) const = 0;
		virtual std::vector<std::shared_ptr<IDockPanel>> GetPanels() const = 0;
	};

	class IApplicationWindow
	{
	public:
		virtual ~IApplicationWindow() = default;

		virtual std::string_view GetId() const = 0;
		virtual void SetTitle(std::string _title) = 0;
		virtual std::string GetTitle() const = 0;
		virtual void SetTheme(std::shared_ptr<ITheme> _theme) = 0;
		virtual std::shared_ptr<IMenuBar> GetMenuBar() const = 0;
		virtual std::shared_ptr<IToolBar> AddToolBar(std::string _id) = 0;
		virtual std::shared_ptr<IDockHost> GetDockHost() const = 0;
		virtual void Show() = 0;
		virtual void Close() = 0;
	};
}

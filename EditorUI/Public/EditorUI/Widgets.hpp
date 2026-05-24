#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <EditorUI/Commanding.hpp>
#include <EditorUI/Types.hpp>

namespace cp::editorui
{
	class IContextMenu;
	class IToolBar;

	class IWidget
	{
	public:
		virtual ~IWidget() = default;

		virtual std::string_view GetId() const = 0;
		virtual void SetVisible(bool _visible) = 0;
		virtual bool IsVisible() const = 0;
		virtual void SetEnabled(bool _enabled) = 0;
		virtual bool IsEnabled() const = 0;
	};

	class IContextMenu
	{
	public:
		virtual ~IContextMenu() = default;

		virtual void AddAction(std::shared_ptr<IAction> _action) = 0;
		virtual void AddAction(std::string _label, std::function<void()> _callback) = 0;
		virtual void AddSeparator() = 0;
	};

	class ISearchBox : public IWidget
	{
	public:
		using TextChangedHandler = std::function<void(std::string_view)>;

		virtual void SetPlaceholder(std::string _placeholder) = 0;
		virtual void SetText(std::string _text) = 0;
		virtual std::string GetText() const = 0;
		virtual void SetTextChangedHandler(TextChangedHandler _handler) = 0;
	};

	class ISceneHierarchyView : public IWidget
	{
	public:
		using EntityHandler = std::function<void(EntityId)>;
		using ContextMenuHandler = std::function<std::shared_ptr<IContextMenu>(EntityId)>;

		virtual void SetNodes(std::vector<SceneHierarchyNode> _nodes) = 0;
		virtual void SetSelectedEntity(std::optional<EntityId> _entityId) = 0;
		virtual std::optional<EntityId> GetSelectedEntity() const = 0;
		virtual void SetEntityActivatedHandler(EntityHandler _handler) = 0;
		virtual void SetContextMenuHandler(ContextMenuHandler _handler) = 0;
	};

	class IAssetExplorerView : public IWidget
	{
	public:
		using AssetHandler = std::function<void(const std::filesystem::path&)>;
		using AssetContextMenuHandler = std::function<std::shared_ptr<IContextMenu>(const std::filesystem::path&)>;

		virtual void SetProjectRoot(std::filesystem::path _root) = 0;
		virtual void SetEntries(std::vector<AssetEntry> _entries) = 0;
		virtual std::optional<std::filesystem::path> GetSelectedPath() const = 0;
		virtual void SetAssetSelectedHandler(AssetHandler _handler) = 0;
		virtual void SetAssetOpenedHandler(AssetHandler _handler) = 0;
		virtual void SetContextMenuHandler(AssetContextMenuHandler _handler) = 0;
		virtual void Refresh() = 0;
	};

	class IConsoleView : public IWidget
	{
	public:
		virtual void SetEntries(std::vector<ConsoleEntry> _entries) = 0;
		virtual void SetSearchText(std::string _searchText) = 0;
		virtual void SetMinLevelFilter(LogLevel _minLevel) = 0;
		virtual void Clear() = 0;
	};

	class IInspectorView : public IWidget
	{
	public:
		using FieldEditedHandler = std::function<void(std::string_view, std::string_view, const InspectorField::Value&)>;
		using AddComponentMenuHandler = std::function<std::shared_ptr<IContextMenu>()>;
		using RemoveComponentHandler = std::function<void(std::string_view sectionId)>;

		virtual void SetSections(std::vector<InspectorSection> _sections) = 0;
		virtual void SetFieldEditedHandler(FieldEditedHandler _handler) = 0;
		virtual void SetAddComponentMenuHandler(AddComponentMenuHandler _handler) = 0;
		virtual void SetRemoveComponentHandler(RemoveComponentHandler _handler) = 0;
		virtual void Clear() = 0;
	};

	class IViewportWidget : public IWidget
	{
	public:
		using ResizeHandler = std::function<void(int, int)>;
		using FrameHandler = std::function<void()>;
		using MousePressHandler = std::function<void(const ViewportMouseEvent&)>;
		using MouseReleaseHandler = std::function<void(const ViewportMouseEvent&)>;
		using MouseMoveHandler = std::function<void(int x, int y)>;
		using KeyPressHandler = std::function<void(std::string_view chord)>;
		using KeyReleaseHandler = std::function<void(std::string_view chord)>;

		virtual void SetActiveTool(ViewportTool _tool) = 0;
		virtual ViewportTool GetActiveTool() const = 0;
		virtual void SetOverlayVisible(std::string _overlayId, bool _visible) = 0;
		virtual void* GetNativeRenderSurfaceHandle() const = 0;
		virtual void SetResizeHandler(ResizeHandler _handler) = 0;
		virtual void SetFrameHandler(FrameHandler _handler) = 0;
		virtual void SetMousePressHandler(MousePressHandler _handler) = 0;
		virtual void SetMouseReleaseHandler(MouseReleaseHandler _handler) = 0;
		virtual void SetMouseMoveHandler(MouseMoveHandler _handler) = 0;
		virtual void SetKeyPressHandler(KeyPressHandler _handler) = 0;
		virtual void SetKeyReleaseHandler(KeyReleaseHandler _handler) = 0;
	};

	class ISceneConfigView : public IWidget
	{
	public:
		struct RegistryEntry
		{
			std::string name;
			std::string key;
			bool enabled = false;
			int32_t order = 0;
		};

		struct PassOrderEntry
		{
			std::string key;
			int32_t order = 0;
		};

		using ApplyHandler = std::function<void(
			std::vector<std::string> _enabledPassNames,
			std::vector<std::string> _enabledSystemNames,
			std::vector<PassOrderEntry> _passOrders
		)>;

		using SelectionChangedHandler = std::function<void(std::string_view _key, bool _isPass)>;

		using ParamFieldEditedHandler = std::function<void(
			std::string_view _registryKey,
			bool _isPass,
			std::string_view _sectionId,
			std::string_view _fieldId,
			const InspectorField::Value& _value
		)>;

		virtual void SetRenderPassEntries(std::vector<RegistryEntry> _entries) = 0;
		virtual void SetSystemEntries(std::vector<RegistryEntry> _entries) = 0;
		virtual void SetApplyHandler(ApplyHandler _handler) = 0;
		virtual void SetSelectionChangedHandler(SelectionChangedHandler _handler) = 0;
		virtual void SetParamFieldEditedHandler(ParamFieldEditedHandler _handler) = 0;
		virtual void SetParamSections(std::string_view _title, std::vector<InspectorSection> _sections) = 0;
		virtual void ClearParamSections() = 0;
	};

	class IPluginManagerView : public IWidget
	{
	public:
		struct PluginEntry
		{
			std::string name;
			std::string status; // Idle, Building, Ready or Error
			bool isBusy = false;
		};

		using RecompileHandler = std::function<void(std::string_view pluginName)>;
		using CreatePluginHandler = std::function<void(std::string_view pluginName)>;
		using OpenFolderHandler = std::function<void(std::string_view pluginName)>;

		virtual void SetPlugins(std::vector<PluginEntry> entries) = 0;
		virtual void AppendBuildLine(std::string_view pluginName, std::string_view line) = 0;
		virtual void ClearBuildOutput() = 0;
		virtual void SetRecompileHandler(RecompileHandler handler) = 0;
		virtual void SetCreatePluginHandler(CreatePluginHandler handler) = 0;
		virtual void SetOpenFolderHandler(OpenFolderHandler handler) = 0;
	};

	class IFileDialog : public IWidget
	{
	public:
		enum class Mode
		{
			OpenFile,
			OpenFiles,
			OpenFolder,
			SaveFile
		};

		virtual void SetMode(Mode _mode) = 0;
		virtual Mode GetMode() const = 0;
		virtual void SetTitle(std::string _title) = 0;
		virtual void SetDirectory(std::filesystem::path _directory) = 0;
		virtual void SetFilter(std::string _filter) = 0;
		virtual void SetDefaultFileName(std::string _fileName) = 0;
		virtual std::optional<std::filesystem::path> GetSelectedPath() const = 0;
		virtual std::vector<std::filesystem::path> GetSelectedPaths() const = 0;
		virtual bool Execute() = 0;
	};
}

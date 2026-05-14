#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace cp::editorui
{
	using EntityId = std::uint64_t;

	enum class Orientation : std::uint8_t
	{
		Horizontal,
		Vertical
	};

	enum class DockArea : std::uint8_t
	{
		Left,
		Right,
		Top,
		Bottom,
		Center,
		Floating
	};

	enum class LogLevel : std::uint8_t
	{
		Trace,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	enum class ViewportTool : std::uint8_t
	{
		Select,
		Translate,
		Rotate,
		Scale
	};

	enum class MouseButton : std::uint8_t
	{
		Left,
		Right,
		Middle
	};

	struct ViewportMouseModifiers
	{
		bool alt = false;
		bool ctrl = false;
		bool shift = false;
	};

	struct ViewportMouseEvent
	{
		int x = 0;
		int y = 0;
		MouseButton button = MouseButton::Left;
		ViewportMouseModifiers modifiers;
	};

	enum class ThemeColorRole : std::uint8_t
	{
		WindowBackground,
		PanelBackground,
		PanelBorder,
		TextPrimary,
		TextSecondary,
		Accent,
		Selection,
		Warning,
		Error
	};

	struct ApplicationWindowConfig
	{
		std::string id = "MainWindow";
		std::string title = "Checkpoint Editor";
		int width = 1600;
		int height = 900;
		bool maximized = true;
	};

	struct DockPanelConfig
	{
		std::string id;
		std::string title;
		bool closable = true;
		bool visible = true;
	};

	struct DockPlacement
	{
		DockArea area = DockArea::Center;
		std::string relativeToPanelId;
		bool asTab = false;
	};

	struct SceneHierarchyNode
	{
		EntityId entityId = 0;
		std::string label;
		bool selected = false;
		bool expanded = false;
		std::vector<SceneHierarchyNode> children;
	};

	struct AssetEntry
	{
		std::filesystem::path path;
		bool isDirectory = false;
	};

	struct ConsoleEntry
	{
		LogLevel level = LogLevel::Info;
		std::string channel;
		std::string message;
		std::chrono::system_clock::time_point timestamp{};
	};

	struct InspectorField
	{
		std::string id;
		std::string label;
		enum class ValueType : std::uint8_t
		{
			Bool,
			Int,
			Float,
			String,
			Vec3
		};
		enum class InputType : std::uint8_t
		{
			Default,
			FilePath
		};
		struct Vec3
		{
			double x = 0.0;
			double y = 0.0;
			double z = 0.0;
		};
		using Value = std::variant<bool, std::int64_t, double, std::string, Vec3>;
		ValueType valueType = ValueType::String;
		InputType inputType = InputType::Default;
		Value value = std::string{};
		bool readOnly = true;
	};

	struct InspectorSection
	{
		std::string id;
		std::string title;
		std::vector<InspectorField> fields;
	};

	struct ViewportCreateInfo
	{
		std::string id = "Viewport";
		void* nativeRenderContext = nullptr;
	};

	struct RGBA8
	{
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
		std::uint8_t a = 255;
	};
}

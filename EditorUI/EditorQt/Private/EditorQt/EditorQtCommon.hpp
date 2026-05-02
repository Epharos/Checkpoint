#pragma once

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QResizeEvent>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <EditorUI/EditorUI.hpp>

#include <array>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp::editorqt
{
	inline QString ToQString(const std::string_view _value)
	{
		return QString::fromUtf8(_value.data(), static_cast<int>(_value.size()));
	}

	inline std::string ToStdString(const QString& _value)
	{
		return _value.toStdString();
	}

	inline QString ColorToCss(const cp::editorui::RGBA8 _color)
	{
		return QString("rgba(%1, %2, %3, %4)")
			.arg(_color.r)
			.arg(_color.g)
			.arg(_color.b)
			.arg(_color.a);
	}

	inline Qt::DockWidgetArea ToQtDockArea(const cp::editorui::DockArea _area)
	{
		switch (_area)
		{
		case cp::editorui::DockArea::Left: return Qt::LeftDockWidgetArea;
		case cp::editorui::DockArea::Right: return Qt::RightDockWidgetArea;
		case cp::editorui::DockArea::Top: return Qt::TopDockWidgetArea;
		case cp::editorui::DockArea::Bottom: return Qt::BottomDockWidgetArea;
		case cp::editorui::DockArea::Floating: return Qt::NoDockWidgetArea;
		case cp::editorui::DockArea::Center:
		default: return Qt::NoDockWidgetArea;
		}
	}

	inline Qt::Orientation ToQtOrientation(const cp::editorui::DockArea _area)
	{
		return (_area == cp::editorui::DockArea::Top || _area == cp::editorui::DockArea::Bottom)
			       ? Qt::Vertical
			       : Qt::Horizontal;
	}

	inline QString LogLevelToQString(const cp::editorui::LogLevel _level)
	{
		switch (_level)
		{
		case cp::editorui::LogLevel::Trace: return "Trace";
		case cp::editorui::LogLevel::Debug: return "Debug";
		case cp::editorui::LogLevel::Info: return "Info";
		case cp::editorui::LogLevel::Warning: return "Warning";
		case cp::editorui::LogLevel::Error: return "Error";
		case cp::editorui::LogLevel::Fatal: return "Fatal";
		default: return "Unknown";
		}
	}

	inline int LogLevelToIndex(const cp::editorui::LogLevel _level)
	{
		return static_cast<int>(_level);
	}

	inline cp::editorui::LogLevel IndexToLogLevel(const int _index)
	{
		if (_index < 0 || _index > static_cast<int>(cp::editorui::LogLevel::Fatal))
		{
			return cp::editorui::LogLevel::Info;
		}
		return static_cast<cp::editorui::LogLevel>(_index);
	}

	class IQtActionAccess
	{
	public:
		virtual ~IQtActionAccess() = default;
		virtual QAction* GetQAction() const = 0;
	};

	class IQtWidgetAccess
	{
	public:
		virtual ~IQtWidgetAccess() = default;
		virtual QWidget* GetQWidget() const = 0;
	};

	class QtWidgetBase
	{
	public:
		explicit QtWidgetBase(std::string _id)
			: id(std::move(_id))
		{
		}

		virtual ~QtWidgetBase() = default;

		[[nodiscard]] std::string_view GetId() const
		{
			return id;
		}

		void SetVisible(const bool _visible) const
		{
			GetQWidget()->setVisible(_visible);
		}

		[[nodiscard]] bool IsVisible() const
		{
			return GetQWidget()->isVisible();
		}

		void SetEnabled(const bool _enabled) const
		{
			GetQWidget()->setEnabled(_enabled);
		}

		[[nodiscard]] bool IsEnabled() const
		{
			return GetQWidget()->isEnabled();
		}

		[[nodiscard]] virtual QWidget* GetQWidget() const = 0;

	private:
		std::string id;
	};
}


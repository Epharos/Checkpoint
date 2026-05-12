#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtViewportSurface final : public QWidget
	{
	public:
		explicit QtViewportSurface(QWidget* _parent = nullptr)
			: QWidget(_parent)
		{
			setAutoFillBackground(true);
			QPalette palette = this->palette();
			palette.setColor(QPalette::Window, QColor(22, 22, 26));
			setPalette(palette);
			setMouseTracking(true);
		}

		std::function<void(int, int)> resizeHandler;
		std::function<void(const cp::editorui::ViewportMouseEvent&)> mousePressHandler;
		std::function<void(const cp::editorui::ViewportMouseEvent&)> mouseReleaseHandler;
		std::function<void(int, int)> mouseMoveHandler;

	protected:
		void resizeEvent(QResizeEvent* _event) override
		{
			QWidget::resizeEvent(_event);
			if (resizeHandler)
			{
				resizeHandler(_event->size().width(), _event->size().height());
			}
		}

		void mousePressEvent(QMouseEvent* _event) override
		{
			QWidget::mousePressEvent(_event);
			if (mousePressHandler)
			{
				mousePressHandler(ToViewportMouseEvent(_event));
			}
		}

		void mouseReleaseEvent(QMouseEvent* _event) override
		{
			QWidget::mouseReleaseEvent(_event);
			if (mouseReleaseHandler)
			{
				mouseReleaseHandler(ToViewportMouseEvent(_event));
			}
		}

		void mouseMoveEvent(QMouseEvent* _event) override
		{
			QWidget::mouseMoveEvent(_event);
			if (mouseMoveHandler)
			{
				mouseMoveHandler(_event->pos().x(), _event->pos().y());
			}
		}

	private:
		static cp::editorui::ViewportMouseEvent ToViewportMouseEvent(const QMouseEvent* _event)
		{
			cp::editorui::MouseButton button = cp::editorui::MouseButton::Left;
			if (_event->button() == Qt::RightButton)
				button = cp::editorui::MouseButton::Right;
			else if (_event->button() == Qt::MiddleButton)
				button = cp::editorui::MouseButton::Middle;
			return { _event->pos().x(), _event->pos().y(), button };
		}
	};

	class QtViewportWidget final : public cp::editorui::IViewportWidget, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtViewportWidget(std::string _id)
			: QtWidgetBase(std::move(_id)),
			  surface(new QtViewportSurface()),
			  frameTimer(new QTimer(surface.get()))
		{
			surface->setObjectName(ToQString(GetId()));
			surface->setMinimumSize(320, 180);

			frameTimer->setInterval(16);
			QObject::connect(frameTimer.get(), &QTimer::timeout, [this]()
			{
				if (frameHandler)
				{
					frameHandler();
				}
			});
			frameTimer->start();
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(const bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(const bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetActiveTool(const cp::editorui::ViewportTool _tool) override
		{
			activeTool = _tool;
		}

		[[nodiscard]] cp::editorui::ViewportTool GetActiveTool() const override
		{
			return activeTool;
		}

		void SetOverlayVisible(std::string _overlayId, const bool _visible) override
		{
			overlayVisibility[std::move(_overlayId)] = _visible;
		}

		[[nodiscard]] void* GetNativeRenderSurfaceHandle() const override
		{
			return reinterpret_cast<void*>(surface->winId());
		}

		void SetResizeHandler(ResizeHandler _handler) override
		{
			surface->resizeHandler = std::move(_handler);
		}

		void SetFrameHandler(FrameHandler _handler) override
		{
			frameHandler = std::move(_handler);
		}

		void SetMousePressHandler(MousePressHandler _handler) override
		{
			surface->mousePressHandler = std::move(_handler);
		}

		void SetMouseReleaseHandler(MouseReleaseHandler _handler) override
		{
			surface->mouseReleaseHandler = std::move(_handler);
		}

		void SetMouseMoveHandler(MouseMoveHandler _handler) override
		{
			surface->mouseMoveHandler = std::move(_handler);
		}

		[[nodiscard]] QWidget* GetQWidget() const override
		{
			return surface.get();
		}

	private:
		std::unique_ptr<QtViewportSurface> surface;
		std::unique_ptr<QTimer> frameTimer;
		FrameHandler frameHandler;
		cp::editorui::ViewportTool activeTool = cp::editorui::ViewportTool::Select;
		std::unordered_map<std::string, bool> overlayVisibility;
	};
}


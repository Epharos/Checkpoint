module;

#include <iostream>
#include "../macros.hpp"

#include "QtWidgets/DraggableDock.hpp"
#include "QtWidgets/DropMainWindow.hpp"

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qboxlayout.h>

//TMP
#include <QtWidgets/qlabel.h>

export module EditorUI:Window;

import :Core;

export namespace cp {
	enum class DockArea { Left, Right, Top, Bottom, Tab };

	class IWindow : public IWidget {
		public:
			EDITOR_API virtual void SetTitle(const std::string& title) noexcept = 0;
			EDITOR_API virtual std::string GetTitle() const noexcept = 0;

			EDITOR_API virtual void SetSize(const unsigned int width, const unsigned int height) noexcept = 0;
			EDITOR_API virtual void GetSize(unsigned int& width, unsigned int& height) const noexcept = 0;

			EDITOR_API virtual void Show() noexcept = 0;
			EDITOR_API virtual void Close() noexcept = 0;

			EDITOR_API virtual void SetContainer(IContainer* container) noexcept = 0;
			EDITOR_API virtual IContainer* GetContainer() const noexcept = 0;

			EDITOR_API virtual void MatchSizeToContent() noexcept = 0;
	};

	class IDockableWindow : public IWindow {
		public:
			EDITOR_API virtual void DockTo(IDockableWindow* target, DockArea area) = 0;
			
	};
}

// -- QT -- //

export namespace cp {
	class QtWindow : public IWindow {
		public:
			EDITOR_API QtWindow() {
				mainWindow = new DropMainWindow();
				mainWindow->setWindowTitle(QString::fromStdString("Checkpoint editor window"));
				mainWindow->resize(480, 360);
			}

			EDITOR_API virtual ~QtWindow() {
				delete mainWindow;
			}

			EDITOR_API virtual void SetTitle(const std::string& _title) noexcept override {
				mainWindow->setWindowTitle(QString::fromStdString(_title));
			}

			EDITOR_API virtual std::string GetTitle() const noexcept override {
				return mainWindow->windowTitle().toStdString();
			}

			EDITOR_API virtual void SetSize(const unsigned int _width, const unsigned int _height) noexcept override {
				mainWindow->resize(_width, _height);
			}

			EDITOR_API virtual void GetSize(unsigned int& _width, unsigned int& _height) const noexcept override {
				QSize size = mainWindow->size();
				_width = size.width();
				_height = size.height();
			}

			EDITOR_API virtual void Show() noexcept override {
				mainWindow->show();
			}

			EDITOR_API virtual void Close() noexcept override {
				mainWindow->close();
			}

			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(mainWindow);
			}

			EDITOR_API virtual void SetVisible(bool _visible) noexcept override {
				mainWindow->setVisible(_visible);
			}

			EDITOR_API virtual bool IsVisible() const noexcept override {
				return mainWindow->isVisible();
			}

			EDITOR_API virtual void SetEnabled(bool _enabled) noexcept override {
				mainWindow->setEnabled(_enabled);
			}

			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return mainWindow->isEnabled();
			}

			EDITOR_API virtual void SetContainer(IContainer* _container) noexcept {
				if(container == _container) {
					return;
				}
				
				if (container) {
					if (auto native = container->NativeHandle()) {
						QWidget* widget = reinterpret_cast<QWidget*>(native);
						widget->setParent(nullptr);
					}
				}

				container = _container;

				if (!container) {
					mainWindow->setCentralWidget(content);
					return;
				}

				QWidget* native = nullptr;
				native = reinterpret_cast<QWidget*>(container->NativeHandle());

				if (native) {
					native->setParent(mainWindow);
					mainWindow->setCentralWidget(native);
					return;
				}

				mainWindow->setCentralWidget(content);
			}

			EDITOR_API virtual IContainer* GetContainer() const noexcept {
				return container;
			}

			EDITOR_API DropMainWindow* MainWindow() const noexcept {
				return mainWindow;
			}

			EDITOR_API void MatchSizeToContent() noexcept override {
				if (container) {
					QWidget* native = reinterpret_cast<QWidget*>(container->NativeHandle());

					if (native) {
						mainWindow->resize(native->sizeHint());
						return;
					}
				}

				if (content)
					mainWindow->resize(content->sizeHint());
			}

		protected:
			DropMainWindow* mainWindow = nullptr;
			QWidget* content = nullptr;
			IContainer* container = nullptr;
	};

	class QtDockableWindow : public IDockableWindow {
		public:
			EDITOR_API QtDockableWindow(DropMainWindow* initialHost = nullptr) {
				host = initialHost;

				if (!host) {
					host = new DropMainWindow();
					host->setWindowTitle(QString::fromStdString("Checkpoint editor window"));
					host->resize(640, 480);
					ownsHost = true;
				}

				content = new QWidget();
				QVBoxLayout* layout = new QVBoxLayout(content);
				layout->setContentsMargins(0, 0, 0, 0);
				layout->setSpacing(0);
				content->setLayout(layout);

				dock = new DraggableDockWidget("Dockable", host);
				dock->setAllowedAreas(Qt::AllDockWidgetAreas);
				dock->setWidget(content);
				dock->setStyleSheet(R"(
					QDockWidget { 
						background: #1A1F2B; 
						padding: 0; 
						margin: 0; 
						border: none; 
					} 

					QWidget { 
						background: #292A38; 
						border: none; 
						padding: 0; 
						margin: 0; 
					})");

				host->addDockWidget(Qt::RightDockWidgetArea, dock);
				host->setStyleSheet(R"(
					QMainWindow { 
						background: #1A1F2B; 
						padding: 0; 
						margin: 0; 
						border: none; 
					} 

					QWidget { 
						background: #1A1F2B; 
						border: none; 
						padding: 0; 
						margin: 0; 
					})");
				dock->show();
			}

			EDITOR_API virtual ~QtDockableWindow() override {
				if (ownsHost) {
					delete host;
				} else {
					host->removeDockWidget(dock);
				}
			}

			EDITOR_API virtual void SetTitle(const std::string& title) noexcept override {
				dock->setWindowTitle(QString::fromStdString(title));
				static_cast<DragTitleBar*>(dock->titleBarWidget())->SetLabel(QString::fromStdString(title));
			}

			EDITOR_API virtual std::string GetTitle() const noexcept override {
				return dock->windowTitle().toStdString();
			}

			EDITOR_API virtual void SetSize(const unsigned int width, const unsigned int height) noexcept override {
				dock->window()->resize(width, height);
			}

			EDITOR_API virtual void GetSize(unsigned int& width, unsigned int& height) const noexcept override {
				QSize size = dock->window()->size();
				width = size.width();
				height = size.height();
			}

			EDITOR_API virtual void Show() noexcept override {
				dock->show();
			}

			EDITOR_API virtual void Close() noexcept override {
				dock->close();
			}

			EDITOR_API virtual void DockTo(IDockableWindow* target, DockArea area) noexcept override {
				if (!target) return;

				QtDockableWindow* qtTarget = dynamic_cast<QtDockableWindow*>(target);
				if (!qtTarget) return;

				DropMainWindow* targetHost = qtTarget->Host();
				if (!targetHost) return;

				if (dock->parentWidget() && dock->parentWidget() != targetHost) {
					dock->setParent(nullptr);
				}

				if (area == DockArea::Tab) {
					targetHost->addDockWidget(Qt::LeftDockWidgetArea, dock);
					targetHost->tabifyDockWidget(qtTarget->Dock(), dock);
					dock->show();
				}
				else {
					Qt::Orientation orientation = (area == DockArea::Left || area == DockArea::Right) ? Qt::Orientation::Horizontal : Qt::Orientation::Vertical;
					targetHost->addDockWidget(ToQtDockArea(area), dock, orientation);
					dock->show();
				}

				host = targetHost;
			}

			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(dock);
			}

			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				dock->setVisible(visible);
			}

			EDITOR_API virtual bool IsVisible() const noexcept override {
				return dock->isVisible();
			}

			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				dock->setEnabled(enabled);
			}

			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return dock->isEnabled();
			}

			[[nodiscard]] EDITOR_API inline QWidget* ContentWidget() noexcept { return content; }
			[[nodiscard]] EDITOR_API inline DropMainWindow* Host() const noexcept { return host; }
			[[nodiscard]] EDITOR_API inline DraggableDockWidget* Dock() const noexcept { return dock; }

			EDITOR_API virtual void SetContainer(IContainer* _container) noexcept {
				if (container == _container) {
					return;
				}

				if (container) {
					if (auto native = container->NativeHandle()) {
						QWidget* widget = reinterpret_cast<QWidget*>(native);
						widget->setParent(nullptr);
						dock->setWidget(nullptr);
					}
				}

				container = _container;

				if (!container) {
					dock->setWidget(content);
					return;
				}

				QWidget* native = nullptr;
				native = reinterpret_cast<QWidget*>(container->NativeHandle());

				if (native) {
					dock->setWidget(native);
					return;
				}

				dock->setWidget(content);
			}

			EDITOR_API virtual IContainer* GetContainer() const noexcept {
				return container;
			}

			EDITOR_API void MatchSizeToContent() noexcept override {
				if (container) {
					QWidget* native = reinterpret_cast<QWidget*>(container->NativeHandle());

					if (native) {
						host->resize(native->sizeHint());
						return;
					}
				}

				if (content)
					host->resize(content->sizeHint());
			}

		private:
			DropMainWindow* host = nullptr;
			DraggableDockWidget* dock = nullptr;
			QWidget* content;
			IContainer* container = nullptr;

			bool ownsHost = false;

			static Qt::DockWidgetArea ToQtDockArea(DockArea area) {
				switch (area) {
					case DockArea::Left: return Qt::LeftDockWidgetArea;
					case DockArea::Right: return Qt::RightDockWidgetArea;
					case DockArea::Top: return Qt::TopDockWidgetArea;
					case DockArea::Bottom: return Qt::BottomDockWidgetArea;
					case DockArea::Tab: return Qt::NoDockWidgetArea;
					default: return Qt::LeftDockWidgetArea;
				}
			}
	};
}
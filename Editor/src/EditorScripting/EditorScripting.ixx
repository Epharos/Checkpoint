module;

#include "../macros.hpp"
#include <iostream>

export module EditorScripting;

export import :Util;
export import :Core;
export import :Window;

export namespace cp {
	class IEditorWidgetFactory {
		public:
			EDITOR_API virtual std::unique_ptr<IContainer> CreateContainer() noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IWindow> CreateWindow() noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IDockableWindow> CreateDockableWindow(IWindow* target = nullptr) noexcept = 0;
	};

	class QtEditorWidgetFactory : public IEditorWidgetFactory {
		public:
			EDITOR_API virtual std::unique_ptr<IContainer> CreateContainer() noexcept override {
				return std::make_unique<cp::QtContainer>();
			}

			EDITOR_API virtual std::unique_ptr<IWindow> CreateWindow() noexcept override {
				return std::make_unique<cp::QtWindow>();
			}

			EDITOR_API virtual std::unique_ptr<IDockableWindow> CreateDockableWindow(IWindow* target = nullptr) noexcept override {
				QMainWindow* host = nullptr;
				if (target) {
					QtWindow* qtTarget = dynamic_cast<QtWindow*>(target);
					if (qtTarget) {
						host = qtTarget->MainWindow();
					}
				}

				return std::make_unique<cp::QtDockableWindow>(host);
			}
	};
}
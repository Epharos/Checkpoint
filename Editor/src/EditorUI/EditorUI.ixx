module;

#include "../macros.hpp"
#include "QtWidgets/DropMainWindow.hpp"
#include <iostream>

export module EditorUI;

export import :Util;
export import :Core;
export import :Window;
export import :Private;

export namespace cp {
	class IEditorUIFactory {
		public:
			EDITOR_API virtual std::unique_ptr<IContainer> CreateContainer() noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IWindow> CreateWindow() noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IDockableWindow> CreateDockableWindow(IWindow* target = nullptr) noexcept = 0;

			virtual std::unique_ptr<ISceneHierarchy> CreateSceneHierarchy() noexcept = 0;
	};

	class QtEditorUIFactory : public IEditorUIFactory {
		public:
			EDITOR_API virtual std::unique_ptr<IContainer> CreateContainer() noexcept override {
				return std::make_unique<cp::QtContainer>();
			}

			EDITOR_API virtual std::unique_ptr<IWindow> CreateWindow() noexcept override {
				return std::make_unique<cp::QtWindow>();
			}

			EDITOR_API virtual std::unique_ptr<IDockableWindow> CreateDockableWindow(IWindow* target = nullptr) noexcept override {
				DropMainWindow* host = nullptr;
				if (target) {
					QtWindow* qtTarget = dynamic_cast<QtWindow*>(target);
					if (qtTarget) {
						host = qtTarget->MainWindow();
					}
				}

				return std::make_unique<cp::QtDockableWindow>(host);
			}

			virtual std::unique_ptr<ISceneHierarchy> CreateSceneHierarchy() noexcept {
				return std::make_unique<QtSceneHierarchy>();
			}
	};
}
module;

#include "../macros.hpp"
#include "QtWidgets/DropMainWindow.hpp"
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

export module EditorUI;

export import :Util;
export import :Core;
export import :Primitive;
export import :Window;
export import :Private;

export namespace cp {
	class IEditorUIFactory {
		public:
			EDITOR_API virtual std::unique_ptr<IContainer> CreateContainer() noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IWindow> CreateWindow() noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IDockableWindow> CreateDockableWindow(IWindow* target = nullptr) noexcept = 0;

			virtual std::unique_ptr<ISceneHierarchy> CreateSceneHierarchy() noexcept = 0;
			virtual std::unique_ptr<IInspector> CreateInspector() noexcept = 0;

			EDITOR_API virtual std::unique_ptr<ILabel> CreateLabel(const std::string& text = "") noexcept = 0;
			EDITOR_API virtual std::unique_ptr<ICollapsible> CreateCollapsible() noexcept = 0;

			EDITOR_API virtual std::unique_ptr<INumericField<int>> CreateIntField(int* value) noexcept = 0;
			EDITOR_API virtual std::unique_ptr<INumericField<float>> CreateFloatField(float* value) noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IVectorField<float, 2>> CreateFloat2Field(void* data, const std::string& labelName) noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IVectorField<float, 3>> CreateFloat3Field(void* data, const std::string& labelName) noexcept = 0;
			EDITOR_API virtual std::unique_ptr<IVectorField<float, 4>> CreateQuaternionField(void* data, const std::string& labelName) noexcept = 0;

			EDITOR_API virtual std::unique_ptr<IFileSelector> CreateFileSelector(std::string* value, const std::string& label, const std::vector<std::string>& extensions) noexcept = 0;
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

			virtual std::unique_ptr<IInspector> CreateInspector() noexcept {
				return std::make_unique<QtInspector>();
			}

			EDITOR_API virtual std::unique_ptr<ILabel> CreateLabel(const std::string& text = "") noexcept override {
				return std::make_unique<cp::QtLabel>(text);
			}

			EDITOR_API virtual std::unique_ptr<ICollapsible> CreateCollapsible() noexcept override {
				return std::make_unique<cp::QtCollapsible>();
			}

			EDITOR_API virtual std::unique_ptr<INumericField<int>> CreateIntField(int* value) noexcept override {
				return std::make_unique<cp::QtNumericField<int>>(value);
			}

			EDITOR_API virtual std::unique_ptr<INumericField<float>> CreateFloatField(float* value) noexcept override {
				return std::make_unique<cp::QtNumericField<float>>(value);
			}

			EDITOR_API virtual std::unique_ptr<IVectorField<float, 2>> CreateFloat2Field(void* data, const std::string& labelName) noexcept override {
				static auto getter = [](void* ptr, int index) -> float& {
					return (*reinterpret_cast<glm::vec2*>(ptr))[index];
				};
				
				return std::make_unique<cp::QtVectorField<float, 2>>(data, getter, labelName);
			}

			EDITOR_API virtual std::unique_ptr<IVectorField<float, 3>> CreateFloat3Field(void* data, const std::string& labelName) noexcept override {
				static auto getter = [](void* ptr, int index) -> float& {
					return (*reinterpret_cast<glm::vec3*>(ptr))[index];
				};
				
				return std::make_unique<cp::QtVectorField<float, 3>>(data, getter, labelName);
			}

			EDITOR_API virtual std::unique_ptr<IVectorField<float, 4>> CreateQuaternionField(void* data, const std::string& labelName) noexcept override {
				static auto getter = [](void* ptr, int index) -> float& {
					return (*reinterpret_cast<glm::quat*>(ptr))[index];
				};
				
				return std::make_unique<cp::QtVectorField<float, 4>>(data, getter, labelName);
			}

			EDITOR_API virtual std::unique_ptr<IFileSelector> CreateFileSelector(std::string* value, const std::string& label, const std::vector<std::string>& extensions) noexcept override {
				return std::make_unique<cp::QtFileSelector>(label, value, extensions);
			}
	};
}
module;

#include "../macros.hpp"

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>

export module EditorUI:Core;

import :Util;

export namespace cp {
	class IWidget {
		public:
			EDITOR_API virtual ~IWidget() = default;

			EDITOR_API virtual void  SetVisible(bool visible) noexcept = 0;
			EDITOR_API virtual bool IsVisible() const noexcept = 0;

			EDITOR_API virtual void SetEnabled(bool enabled) noexcept = 0;
			EDITOR_API virtual bool IsEnabled() const noexcept = 0;

			EDITOR_API virtual void* NativeHandle() const noexcept = 0;
	};

	class IContainer : public IWidget {
		public:
			EDITOR_API virtual ~IContainer() = default;

			EDITOR_API virtual void AddChild(IWidget* child) noexcept = 0;
			EDITOR_API virtual void RemoveChild(IWidget* child) noexcept = 0;
			EDITOR_API virtual void ClearChildren() noexcept = 0;
	};
}

// -- QT -- //

export namespace cp {
	class QtContainer : public IContainer {
		public:
			EDITOR_API QtContainer() {
				containerWidget = new QWidget();
				layout = new QVBoxLayout(containerWidget);
				layout->setContentsMargins(0, 0, 0, 0);
				layout->setSpacing(0);
				containerWidget->setLayout(layout);
			}

			EDITOR_API virtual ~QtContainer() override {
				delete containerWidget;
			}

			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				containerWidget->setVisible(visible);
			}

			EDITOR_API virtual bool IsVisible() const noexcept override {
				return containerWidget->isVisible();
			}

			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				containerWidget->setEnabled(enabled);
			}

			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return containerWidget->isEnabled();
			}

			EDITOR_API virtual void AddChild(IWidget* child) noexcept override {
				if (!child) return;

				if (void* native = child->NativeHandle()) {
					QWidget* childWidget = reinterpret_cast<QWidget*>(native);
					layout->addWidget(childWidget);
					children.push_back(child);
				}
			}

			EDITOR_API virtual void RemoveChild(IWidget* child) noexcept override {
				if (!child) return;

				if (void* native = child->NativeHandle()) {
					QWidget* childWidget = reinterpret_cast<QWidget*>(native);
					layout->removeWidget(childWidget);
					childWidget->setParent(nullptr);
				}

				children.erase(std::remove(children.begin(), children.end(), child), children.end());
			}

			EDITOR_API virtual void ClearChildren() noexcept override {
				for (auto child : children) {
					if (void* native = child->NativeHandle()) {
						QWidget* childWidget = reinterpret_cast<QWidget*>(native);
						layout->removeWidget(childWidget);
						childWidget->setParent(nullptr);
					}
				}

				children.clear();
			}

			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(containerWidget);
			}

	protected:
		QWidget* containerWidget = nullptr;
		QVBoxLayout* layout = nullptr;
		std::vector<IWidget*> children;
	};
}
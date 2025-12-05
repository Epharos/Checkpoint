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
			EDITOR_API IContainer(ContainerOrientation orient = ContainerOrientation::Vertical)
				: orientation(orient) {
			}
			EDITOR_API virtual ~IContainer() = default;

			EDITOR_API virtual void AddChild(IWidget* child) noexcept = 0;
			EDITOR_API virtual void RemoveChild(IWidget* child) noexcept = 0;
			EDITOR_API virtual void ClearChildren() noexcept = 0;

			EDITOR_API virtual void SetSpacing(int spacing) noexcept = 0;
			EDITOR_API virtual void SetMargins(int left, int top, int right, int bottom) noexcept = 0;

			EDITOR_API virtual void SetFixedWidth(int width) noexcept = 0;
			EDITOR_API virtual void SetFixedHeight(int height) noexcept = 0;
			EDITOR_API virtual void SetFixedSize(int width, int height) noexcept = 0;

			EDITOR_API virtual void SetMinimumWidth(int width) noexcept = 0;
			EDITOR_API virtual void SetMinimumHeight(int height) noexcept = 0;
			EDITOR_API virtual void SetMinimumSize(int width, int height) noexcept = 0;
			EDITOR_API virtual void SetMaximumWidth(int width) noexcept = 0;
			EDITOR_API virtual void SetMaximumHeight(int height) noexcept = 0;
			EDITOR_API virtual void SetMaximumSize(int width, int height) noexcept = 0;

			EDITOR_API virtual const std::vector<IWidget*>& GetChildren() const noexcept {
				return children;
			}

			EDITOR_API virtual size_t ChildCount() const noexcept {
				return children.size();
			}

			EDITOR_API virtual void SetOrientation(ContainerOrientation orient) noexcept {
				orientation = orient;
			}

			EDITOR_API ContainerOrientation GetOrientation() const noexcept {
				return orientation;
			}

			EDITOR_API virtual void SetVertical() noexcept {
				SetOrientation(ContainerOrientation::Vertical);
			}

			EDITOR_API virtual void SetHorizontal() noexcept {
				SetOrientation(ContainerOrientation::Horizontal);
			}

		protected:
			std::vector<IWidget*> children;

			ContainerOrientation orientation;
	};
}

// -- QT -- //

export namespace cp {
	class QtContainer : public IContainer {
		public:
			EDITOR_API QtContainer(ContainerOrientation orientation = ContainerOrientation::Vertical) : IContainer(orientation) {
				containerWidget = new QWidget();
				
				switch (orientation) {
				case ContainerOrientation::Vertical:
					layout = new QVBoxLayout(containerWidget);
					break;
				case ContainerOrientation::Horizontal:
					layout = new QHBoxLayout(containerWidget);
					break;
				}

				layout->setContentsMargins(margins[0], margins[1], margins[2], margins[3]);
				layout->setSpacing(spacing);
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

			EDITOR_API virtual void SetOrientation(ContainerOrientation orient) noexcept {
				IContainer::SetOrientation(orient);

				delete layout;

				switch(orientation) {
					case ContainerOrientation::Vertical:
						layout = new QVBoxLayout(containerWidget);
						break;
					case ContainerOrientation::Horizontal:
						layout = new QHBoxLayout(containerWidget);
						break;
				}

				layout->setContentsMargins(margins[0], margins[1], margins[2], margins[3]);
				layout->setSpacing(spacing);
				containerWidget->setLayout(layout);

				for (auto child : children) {
					if (void* native = child->NativeHandle()) {
						QWidget* childWidget = reinterpret_cast<QWidget*>(native);
						layout->addWidget(childWidget);
					}
				}
			}

			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(containerWidget);
			}

			EDITOR_API virtual void SetSpacing(int spacing) noexcept override {
				layout->setSpacing(spacing);
				this->spacing = spacing;
			}

			EDITOR_API virtual void SetMargins(int left, int top, int right, int bottom) noexcept override {
				layout->setContentsMargins(left, top, right, bottom);
				margins[0] = left;
				margins[1] = top;
				margins[2] = right;
				margins[3] = bottom;
			}

			EDITOR_API virtual void SetFixedWidth(int width) noexcept override
			{
				if (width < 0)
				{
					containerWidget->setSizePolicy(QSizePolicy::Expanding, containerWidget->sizePolicy().verticalPolicy());
					return;
				}

				containerWidget->setFixedWidth(width);
			}

			EDITOR_API virtual void SetFixedHeight(int height) noexcept override
			{
				if (height < 0)
				{
					containerWidget->setSizePolicy(containerWidget->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
					return;
				}

				containerWidget->setFixedHeight(height);
			}

			EDITOR_API virtual void SetFixedSize(int width, int height) noexcept override
			{
				SetFixedWidth(width);
				SetFixedHeight(height);
			}

			EDITOR_API virtual void SetMinimumWidth(int width) noexcept override
			{
				containerWidget->setMinimumWidth(width);
			}

			EDITOR_API virtual void SetMinimumHeight(int height) noexcept override
			{
				containerWidget->setMinimumHeight(height);
			}

			EDITOR_API virtual void SetMinimumSize(int width, int height) noexcept override
			{
				SetMinimumWidth(width);
				SetMinimumHeight(height);
			}

			EDITOR_API virtual void SetMaximumWidth(int width) noexcept override
			{
				containerWidget->setMaximumWidth(width);
			}

			EDITOR_API virtual void SetMaximumHeight(int height) noexcept override
			{
				containerWidget->setMaximumHeight(height);
			}

			EDITOR_API virtual void SetMaximumSize(int width, int height) noexcept override
			{
				SetMaximumWidth(width);
				SetMaximumHeight(height);
			}

	protected:
		QWidget* containerWidget = nullptr;
		QBoxLayout* layout = nullptr;
		int margins[4] = { 0, 0, 0, 0 };
		int spacing = 0;
	};
}
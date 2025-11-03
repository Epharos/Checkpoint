#pragma once

#include "../../pch.hpp"

import EditorUI:Core;

namespace cp {
	class ICollapsible : public IWidget {
	public:
		virtual ~ICollapsible() = default;
		virtual void SetContent(IWidget* content) noexcept = 0;
		virtual void SetContentVisible(bool visible) noexcept = 0;
		virtual bool IsContentVisible() const noexcept = 0;

		virtual void SetTitle(const std::string& title, std::optional<std::string> icon = std::nullopt) noexcept = 0;
	};
}

// -- QT -- //

namespace cp {
	class QtCollapsible : public ICollapsible {
	public:
		EDITOR_API QtCollapsible() {
			collapsibleWidget = new QWidget();

			QVBoxLayout* layout = new QVBoxLayout(collapsibleWidget);
			layout->setSpacing(0);
			layout->setContentsMargins(0, 0, 0, 0);

			toggleButton = new QToolButton(collapsibleWidget);
			toggleButton->setText(title);
			toggleButton->setCheckable(true);
			toggleButton->setChecked(_expanded);
			toggleButton->setStyleSheet("QToolButton { border: none; font-weight: bold; }");
			toggleButton->setArrowType(Qt::DownArrow);
			toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			toggleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			contentArea = new QFrame(collapsibleWidget);
			contentArea->setFrameShape(QFrame::NoFrame);
			contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			contentLayout = new QVBoxLayout(contentArea);
			contentLayout->setContentsMargins(8, 4, 8, 4);

			contentArea->setVisible(_expanded);
			toggleButton->setArrowType(_expanded ? Qt::DownArrow : Qt::RightArrow);

			layout->addWidget(toggleButton);
			layout->addWidget(contentArea);

			connect(toggleButton, &QToolButton::toggled, this, [&](bool expanded) {
				contentArea->setVisible(expanded);
				toggleButton->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
			});
		}

		EDITOR_API virtual ~QtCollapsible() override {
			delete collapsibleWidget;
		}

		EDITOR_API virtual void SetVisible(bool visible) noexcept override {
			collapsibleWidget->setVisible(visible);
		}

		EDITOR_API virtual bool IsVisible() const noexcept override {
			return collapsibleWidget->isVisible();
		}

		EDITOR_API virtual void SetContent(IWidget* content) noexcept override {
			if (contentLayout->count() > 0) {
				QLayoutItem* item = contentLayout->takeAt(0);
				if (item) {
					delete item->widget();
					delete item;
				}
			}

			contentLayout->addWidget(reinterpret_cast<QWidget*>(content->NativeHandle()));
		}

		EDITOR_API virtual void SetContentVisible(bool visible) noexcept override {
			collapsibleWidget->setVisible(visible);
		}

		EDITOR_API virtual bool IsContentVisible() const noexcept override {
			return collapsibleWidget->isVisible();
		}

		EDITOR_API virtual void SetTitle(const std::string& title, std::optional<std::string> icon = std::nullopt) noexcept override {
			toggleButton->setText(QString::fromStdString(title));
		}
	private:
		QWidget* collapsibleWidget;
		QToolButton* toggleButton;
		QFrame* contentArea;
		QVBoxLayout* contentLayout;
	};
}
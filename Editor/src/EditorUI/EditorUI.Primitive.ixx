module;

#include "../macros.hpp"
#include "QtWidgets/Helper.hpp"

#include <QtWidgets/qlabel.h>
#include <QtCore/qstring.h>

#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qboxlayout.h>

export module EditorUI:Primitive;

import :Core;
import :Util;

export namespace cp {
	class ILabel : public IWidget {
		public:
			EDITOR_API virtual void SetText(const std::string& text) noexcept = 0;
			EDITOR_API virtual std::string GetText() const noexcept = 0;
	};

	class ICollapsible : public IWidget {
	public:
		EDITOR_API virtual ~ICollapsible() = default;
		EDITOR_API virtual void SetContent(IWidget* content) noexcept = 0;
		EDITOR_API virtual void SetContentVisible(bool visible) noexcept = 0;
		EDITOR_API virtual bool IsContentVisible() const noexcept = 0;

		EDITOR_API virtual void SetTitle(const std::string& title, std::optional<std::string> icon = std::nullopt) noexcept = 0;
	};
}

// -- QT -- //

export namespace cp {
	class QtLabel : public ILabel {
		public:
			EDITOR_API QtLabel(const std::string& text = "") {
				labelWidget = new QLabel(QString::fromStdString(text));
			}

			EDITOR_API virtual ~QtLabel() override {
				delete labelWidget;
			}

			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				labelWidget->setVisible(visible);
			}

			EDITOR_API virtual bool IsVisible() const noexcept override {
				return labelWidget->isVisible();
			}

			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				labelWidget->setEnabled(enabled);
			}

			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return labelWidget->isEnabled();
			}

			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(labelWidget);
			}

			EDITOR_API virtual void SetText(const std::string& text) noexcept override {
				labelWidget->setText(QString::fromStdString(text));
			}

			EDITOR_API virtual std::string GetText() const noexcept override {
				return labelWidget->text().toStdString();
			}
		protected:
			QLabel* labelWidget;
	};

	class QtCollapsible : public ICollapsible {
	public:
		EDITOR_API QtCollapsible() {
			collapsibleWidget = new QWidget();

			QVBoxLayout* layout = new QVBoxLayout(collapsibleWidget);
			layout->setSpacing(0);
			layout->setContentsMargins(0, 0, 0, 0);

			toggleButton = new QToolButton(collapsibleWidget);
			toggleButton->setText("Collapsible");
			toggleButton->setCheckable(true);
			toggleButton->setChecked(true);
			toggleButton->setStyleSheet("QToolButton { border: none; font-weight: bold; }");
			toggleButton->setIcon(QIcon(SvgToPixmap("Editor_Resources/Icons/chevron_down.svg", QSize(16, 16), "#D0D3DC")));
			toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			toggleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			contentArea = new QFrame(collapsibleWidget);
			contentArea->setFrameShape(QFrame::NoFrame);
			contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			contentLayout = new QVBoxLayout(contentArea);
			contentLayout->setContentsMargins(8, 4, 8, 4);

			contentArea->setVisible(true);

			layout->addWidget(toggleButton);
			layout->addWidget(contentArea);

			QObject::connect(toggleButton, &QToolButton::toggled, [&](bool expanded) {
				contentArea->setVisible(expanded);
				toggleButton->setIcon(expanded ? 
					QIcon(SvgToPixmap("Editor_Resources/Icons/chevron_down.svg", QSize(16, 16), "#D0D3DC")) : 
					QIcon(SvgToPixmap("Editor_Resources/Icons/chevron_right.svg", QSize(16, 16), "#D0D3DC")));
			});

			collapsibleWidget->setLayout(layout);
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

		EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
			collapsibleWidget->setEnabled(enabled);
		}

		EDITOR_API virtual bool IsEnabled() const noexcept override {
			return collapsibleWidget->isEnabled();
		}

		EDITOR_API virtual void* NativeHandle() const noexcept override {
			return static_cast<void*>(collapsibleWidget);
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
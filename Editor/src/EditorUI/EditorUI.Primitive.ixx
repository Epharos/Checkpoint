module;

#include "../macros.hpp"
#include "../Project.hpp"
#include "QtWidgets/Helper.hpp"

#include <QtWidgets/qlabel.h>
#include <QtCore/qstring.h>
#include <QtGui/qevent.h>

#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlineedit.h>

#include "QtWidgets/FileDropPreviewWidget.hpp"

#include "../CheckpointEditor.hpp"

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

	template<typename T>
	class INumericField : public IWidget {
		public:
			EDITOR_API virtual void SetValue(T value) noexcept = 0;
			EDITOR_API virtual T GetValue() const noexcept = 0;
			EDITOR_API virtual void SetMinValue(T minValue) noexcept = 0;
			EDITOR_API virtual T GetMinValue() const noexcept = 0;
			EDITOR_API virtual void SetMaxValue(T maxValue) noexcept = 0;
			EDITOR_API virtual T GetMaxValue() const noexcept = 0;
			EDITOR_API virtual void SetStep(T step) noexcept = 0;
			EDITOR_API virtual T GetStep() const noexcept = 0;
			EDITOR_API virtual void SetRange(T minValue, T maxValue) noexcept = 0;
			EDITOR_API virtual void SetBorderColor(const uint32_t& color) noexcept = 0;
	};

	template<typename T, int N>
	class IVectorField : public IWidget {
		public:
			EDITOR_API virtual void SetValue(const std::array<T, N>& value) noexcept = 0;
			EDITOR_API virtual std::array<T, N> GetValue() const noexcept = 0;
			EDITOR_API virtual void SetMinValue(const std::array<T, N>& minValue) noexcept = 0;
			EDITOR_API virtual std::array<T, N> GetMinValue() const noexcept = 0;
			EDITOR_API virtual void SetMaxValue(const std::array<T, N>& maxValue) noexcept = 0;
			EDITOR_API virtual std::array<T, N> GetMaxValue() const noexcept = 0;
			EDITOR_API virtual void SetStep(const std::array<T, N>& step) noexcept = 0;
			EDITOR_API virtual std::array<T, N> GetStep() const noexcept = 0;
			EDITOR_API virtual void SetRange(const std::array<T, N>& minValue, const std::array<T, N>& maxValue) noexcept = 0;
	};

	class IStringField : public IWidget {
		public:
			EDITOR_API virtual void SetText(const std::string& text) noexcept = 0;
			EDITOR_API virtual std::string GetText() const noexcept = 0;
	};

	class IFileSelector : public IWidget {
	};

	class IMeshSelector : public IWidget {
	};

	class ITextureSelector : public IWidget {
	};

	class IMaterialInstanceSelector : public IWidget {
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

	template<typename T>
	class QtNumericField : public INumericField<T> {
		public:
			EDITOR_API QtNumericField(T* value)
				: valuePtr(value)
			{
				widget = new QWidget();
				auto layout = new QHBoxLayout(widget);
				layout->setContentsMargins(0, 0, 0, 0);

				minValue = std::numeric_limits<T>::lowest();
				maxValue = std::numeric_limits<T>::max();
				stepValue = static_cast<T>(1);

				lineEdit = new QLineEdit(widget);
				lineEdit->setText(QString::number(*valuePtr));
				lineEdit->setStyleSheet("QLineEdit { padding: 2px 4px; background-color: #1A1F2B; }");
				layout->addWidget(lineEdit);

				widget->setLayout(layout);

				QObject::connect(lineEdit, &QLineEdit::editingFinished, [this]() {
					bool ok = false;
					T v = Parse(lineEdit->text(), &ok);
					if (ok) {
						v = std::clamp(v, minValue, maxValue);
						*valuePtr = v;
						lineEdit->setText(QString::number(v));
					}
					else {
						lineEdit->setText(QString::number(*valuePtr));
					}
					});

				lineEdit->installEventFilter(widget);
			}

			EDITOR_API void SetVisible(bool visible) noexcept override { widget->setVisible(visible); }
			EDITOR_API bool IsVisible() const noexcept override { return widget->isVisible(); }
			EDITOR_API void SetEnabled(bool enabled) noexcept override { widget->setEnabled(enabled); }
			EDITOR_API bool IsEnabled() const noexcept override { return widget->isEnabled(); }
			EDITOR_API void* NativeHandle() const noexcept override { return widget; }

			EDITOR_API void SetValue(T value) noexcept override {
				*valuePtr = std::clamp(value, minValue, maxValue);
				lineEdit->setText(QString::number(*valuePtr));
			}
			EDITOR_API T GetValue() const noexcept override { return *valuePtr; }
			EDITOR_API void SetRange(T min, T max) noexcept override {
				minValue = min; maxValue = max;
				SetValue(*valuePtr);
			}
			EDITOR_API void SetStep(T step) noexcept override { stepValue = step; }
			EDITOR_API T GetStep() const noexcept override { return stepValue; }
			EDITOR_API void SetMinValue(T minValue) noexcept override {
				this->minValue = minValue;
				SetValue(*valuePtr);
			}
			EDITOR_API T GetMinValue() const noexcept override { return minValue; }
			EDITOR_API void SetMaxValue(T maxValue) noexcept override {
				this->maxValue = maxValue;
				SetValue(*valuePtr);
			}
			EDITOR_API T GetMaxValue() const noexcept override { return maxValue; }

			EDITOR_API void SetBorderColor(const uint32_t& color) noexcept override {
				QString style = QString("QLineEdit { padding: 2px 4px; background-color: #1A1F2B; border-left: 3px solid #%1; }")
					.arg(QString::number(color, 16).rightJustified(6, '0'));
				lineEdit->setStyleSheet(style);
			}
		protected:
			QWidget* widget = nullptr;
			QLineEdit* lineEdit = nullptr;

			T* valuePtr = nullptr;
			T minValue, maxValue, stepValue;

			bool dragging = false;
			int lastX = 0;

			bool eventFilter(QObject* obj, QEvent* event) {
				if (obj == lineEdit) {
					if (event->type() == QEvent::MouseButtonPress) {
						dragging = true;
						lastX = static_cast<QMouseEvent*>(event)->globalX();
						return true;
					}
					if (event->type() == QEvent::MouseMove && dragging) {
						int dx = static_cast<QMouseEvent*>(event)->globalX() - lastX;
						if (dx != 0) {
							T v = *valuePtr + dx * stepValue;
							SetValue(v);
							lastX = static_cast<QMouseEvent*>(event)->globalX();
						}
						return true;
					}
					if (event->type() == QEvent::MouseButtonRelease) {
						dragging = false;
						return true;
					}
				}
				return QObject::eventFilter(obj, event);
			}

			EDITOR_API T Parse(const QString& text, bool* ok) const;
	};

	export template<>
	EDITOR_API float QtNumericField<float>::Parse(const QString& text, bool* ok) const {
		return text.toFloat(ok);
	}

	export template<>
	EDITOR_API double QtNumericField<double>::Parse(const QString& text, bool* ok) const {
		return text.toDouble(ok);
	}

	export template<>
	EDITOR_API int QtNumericField<int>::Parse(const QString& text, bool* ok) const {
		return text.toInt(ok);
	}

	export template<>
	EDITOR_API long QtNumericField<long>::Parse(const QString& text, bool* ok) const {
		return text.toLong(ok);
	}

	export template<>
	EDITOR_API long long QtNumericField<long long>::Parse(const QString& text, bool* ok) const {
		return text.toLongLong(ok);
	}

	template<typename T, int N>
	class QtVectorField : public IVectorField<T, N> {
	public:
		using ValueType = T;
		using Getter = std::function<ValueType& (void*, int)>;

		EDITOR_API QtVectorField(void* dataPtr, Getter getter, const std::string& labelTitle)
			: dataPtr(dataPtr), getter(getter)
		{
			widget = new QWidget();
			auto layout = new QHBoxLayout(widget);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(4);

			QLabel* lbl = new QLabel(QString::fromStdString(labelTitle), widget);
			layout->addWidget(lbl);

			for (int i = 0; i < N; ++i) {
				ValueType* compPtr = &getter(dataPtr, i);
				auto* field = new QtNumericField<ValueType>(compPtr);
				fields[i] = field;
				layout->addWidget(static_cast<QWidget*>(field->NativeHandle()));
			}

			if constexpr (N == 2) {
				fields[0]->SetBorderColor(0xFF0000); // Red
				fields[1]->SetBorderColor(0x00FF00); // Green
			}
			else if constexpr (N == 3) {
				fields[0]->SetBorderColor(0xFF0000); // Red
				fields[1]->SetBorderColor(0x00FF00); // Green
				fields[2]->SetBorderColor(0x0000FF); // Blue
			}

			widget->setLayout(layout);
		}

		EDITOR_API void SetVisible(bool visible) noexcept override { widget->setVisible(visible); }
		EDITOR_API bool IsVisible() const noexcept override { return widget->isVisible(); }
		EDITOR_API void SetEnabled(bool enabled) noexcept override { widget->setEnabled(enabled); }
		EDITOR_API bool IsEnabled() const noexcept override { return widget->isEnabled(); }
		EDITOR_API void* NativeHandle() const noexcept override { return widget; }

		EDITOR_API void SetRange(const std::array<ValueType, N>& minValue, const std::array<ValueType, N>& maxValue) noexcept override {
			for (int i = 0; i < N; ++i) fields[i]->SetRange(minValue[i], maxValue[i]);
		}

		EDITOR_API void SetStep(const std::array<ValueType, N>& step) noexcept override {
			for (int i = 0; i < N; ++i) fields[i]->SetStep(step[i]);
		}

		EDITOR_API std::array<ValueType, N> GetStep() const noexcept override {
			std::array<ValueType, N> result;
			for (int i = 0; i < N; ++i) result[i] = fields[i]->GetStep();
			return result;
		}

		EDITOR_API void SetMinValue(const std::array<ValueType, N>& minValue) noexcept override {
			for (int i = 0; i < N; ++i) fields[i]->SetMinValue(minValue[i]);
		}

		EDITOR_API std::array<ValueType, N> GetMinValue() const noexcept override {
			std::array<ValueType, N> result;
			for (int i = 0; i < N; ++i) result[i] = fields[i]->GetMinValue();
			return result;
		}

		EDITOR_API void SetMaxValue(const std::array<ValueType, N>& maxValue) noexcept override {
			for (int i = 0; i < N; ++i) fields[i]->SetMaxValue(maxValue[i]);
		}

		EDITOR_API std::array<ValueType, N> GetMaxValue() const noexcept override {
			std::array<ValueType, N> result;
			for (int i = 0; i < N; ++i) result[i] = fields[i]->GetMaxValue();
			return result;
		}

		EDITOR_API void SetValue(const std::array<ValueType, N>& value) noexcept override {
			for (int i = 0; i < N; ++i) fields[i]->SetValue(value[i]);
		}

		EDITOR_API std::array<ValueType, N> GetValue() const noexcept override {
			std::array<ValueType, N> result;
			for (int i = 0; i < N; ++i) result[i] = fields[i]->GetValue();
			return result;
		}

	private:
		QWidget* widget = nullptr;
		void* dataPtr = nullptr;
		Getter getter;
		std::array<QtNumericField<ValueType>*, N> fields;
	};

	class QtStringField : public IStringField {
		public:
			EDITOR_API QtStringField(std::string* valuePtr, const std::string& labelContent)
				: valuePtr(valuePtr)
			{
				widget = new QWidget();
				auto layout = new QHBoxLayout(widget);
				layout->setContentsMargins(0, 0, 0, 0);
				label = new QLabel(QString::fromStdString(labelContent), widget);
				layout->addWidget(label);
				lineEdit = new QLineEdit(widget);
				lineEdit->setText(QString::fromStdString(*valuePtr));
				lineEdit->setStyleSheet("QLineEdit { padding: 2px 4px; background-color: #1A1F2B; }");
				layout->addWidget(lineEdit);
				widget->setLayout(layout);
				QObject::connect(lineEdit, &QLineEdit::editingFinished, [&]() {
					*valuePtr = lineEdit->text().toStdString();
					});
			}
			EDITOR_API virtual ~QtStringField() override {
				delete widget;
			}
			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				widget->setVisible(visible);
			}
			EDITOR_API virtual bool IsVisible() const noexcept override {
				return widget->isVisible();
			}
			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				widget->setEnabled(enabled);
			}
			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return widget->isEnabled();
			}
			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return widget;
			}
			EDITOR_API virtual void SetText(const std::string& text) noexcept override {
				*valuePtr = text;
				lineEdit->setText(QString::fromStdString(text));
			}
			EDITOR_API virtual std::string GetText() const noexcept override {
				return *valuePtr;
			}
		protected:
			QWidget* widget = nullptr;
			QLabel* label = nullptr;
			QLineEdit* lineEdit = nullptr;
			std::string* valuePtr = nullptr;
	};

	class QtFileSelector : public IFileSelector {
		public:
			EDITOR_API QtFileSelector(const std::string& label, std::string* valuePtr, const std::vector<std::string>& allowedExtensions)
			{
				QStringList extList;

				for (const std::string& ext : allowedExtensions) {
					extList.append(QString::fromStdString(ext));
				}

				widget = new FileDropPreviewWidget(
					valuePtr,
					QString::fromStdString(label),
					extList,
					QString::fromStdString(cp::CheckpointEditor::CurrentProject.GetResourcePath()),
					nullptr
				);
			}

			EDITOR_API virtual ~QtFileSelector() override {
				delete widget;
			}
			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				widget->setVisible(visible);
			}
			EDITOR_API virtual bool IsVisible() const noexcept override {
				return widget->isVisible();
			}
			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				widget->setEnabled(enabled);
			}
			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return widget->isEnabled();
			}
			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(widget);
			}

		protected:
			FileDropPreviewWidget* widget = nullptr;
	};

	class QtMeshSelector : public IMeshSelector {
		public:
			EDITOR_API QtMeshSelector(const std::string& label, std::shared_ptr<cp::Mesh>* valuePtr) {
				widget = new MeshDropPreviewWidget(
					valuePtr,
					QString::fromStdString(label),
					QString::fromStdString(cp::CheckpointEditor::CurrentProject.GetResourcePath()),
					nullptr
				);
			}

			EDITOR_API virtual ~QtMeshSelector() override {
				delete widget;
			}

			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				widget->setVisible(visible);
			}

			EDITOR_API virtual bool IsVisible() const noexcept override {
				return widget->isVisible();
			}

			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				widget->setEnabled(enabled);
			}

			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return widget->isEnabled();
			}

			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(widget);
			}

		protected:
			MeshDropPreviewWidget* widget = nullptr;
	};

	class QtTextureSelector : public ITextureSelector {
		public:
			EDITOR_API QtTextureSelector(const std::string& label, std::shared_ptr<cp::Texture>* valuePtr) {
				widget = new TextureDropPreviewWidget(
					valuePtr,
					QString::fromStdString(label),
					QString::fromStdString(cp::CheckpointEditor::CurrentProject.GetResourcePath()),
					nullptr
				);
			}
			EDITOR_API virtual ~QtTextureSelector() override {
				delete widget;
			}
			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				widget->setVisible(visible);
			}
			EDITOR_API virtual bool IsVisible() const noexcept override {
				return widget->isVisible();
			}
			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				widget->setEnabled(enabled);
			}
			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return widget->isEnabled();
			}
			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(widget);
			}
		protected:
			TextureDropPreviewWidget* widget = nullptr;
	};

	class QtMaterialInstanceSelector : public IMaterialInstanceSelector {
		public:
			EDITOR_API QtMaterialInstanceSelector(const std::string& label, std::shared_ptr<cp::MaterialInstance>* valuePtr) {
				widget = new MaterialInstanceDropPreviewWidget(
					valuePtr,
					QString::fromStdString(label),
					QString::fromStdString(cp::CheckpointEditor::CurrentProject.GetResourcePath()),
					nullptr
				);
			}
			EDITOR_API virtual ~QtMaterialInstanceSelector() override {
				delete widget;
			}
			EDITOR_API virtual void SetVisible(bool visible) noexcept override {
				widget->setVisible(visible);
			}
			EDITOR_API virtual bool IsVisible() const noexcept override {
				return widget->isVisible();
			}
			EDITOR_API virtual void SetEnabled(bool enabled) noexcept override {
				widget->setEnabled(enabled);
			}
			EDITOR_API virtual bool IsEnabled() const noexcept override {
				return widget->isEnabled();
			}
			EDITOR_API virtual void* NativeHandle() const noexcept override {
				return static_cast<void*>(widget);
			}
		protected:
			MaterialInstanceDropPreviewWidget* widget = nullptr;
	};
}
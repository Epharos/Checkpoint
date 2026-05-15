#pragma once

#include "EditorQtCommon.hpp"
#include "QtContextMenu.hpp"
#include "QtFileDropWidget.hpp"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

namespace cp::editorqt
{
	class QtInspectorView final : public cp::editorui::IInspectorView, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtInspectorView(std::string _id)
			: QtWidgetBase(std::move(_id)),
			  mainWidget(new QWidget()),
			  tree(new QTreeWidget()),
			  addComponentButton(new QPushButton("Add Component"))
		{
			tree->setObjectName(ToQString(GetId()));
			tree->setColumnCount(2);
			tree->setHeaderLabels(QStringList{ "Property", "Value" });
			tree->header()->setStretchLastSection(true);
			tree->setContextMenuPolicy(Qt::CustomContextMenu);

			QObject::connect(tree.get(), &QWidget::customContextMenuRequested, [this](const QPoint& _position)
			{
				if (!removeComponentHandler)
				{
					return;
				}

				const QTreeWidgetItem* item = tree->itemAt(_position);
				if (item == nullptr || item->parent() != nullptr)
				{
					return;
				}

				if (!item->data(0, Qt::UserRole + 1).toBool())
				{
					return;
				}

				const std::string sectionId = ToStdString(item->data(0, Qt::UserRole).toString());
				QMenu contextMenu(tree.get());

				const QAction* removeAction = contextMenu.addAction("Remove Component");
				QObject::connect(removeAction, &QAction::triggered, [this, sectionId]()
				{
					if (removeComponentHandler)
					{
						removeComponentHandler(sectionId);
					}
				});

				contextMenu.exec(tree->viewport()->mapToGlobal(_position));
			});

			auto* layout = new QVBoxLayout(mainWidget.get());
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(4);
			layout->addWidget(addComponentButton.get());
			layout->addWidget(tree.get());

			QObject::connect(addComponentButton.get(), &QPushButton::customContextMenuRequested, [this](const QPoint& _position)
			{
				ShowAddComponentMenu(_position);
			});

			QObject::connect(addComponentButton.get(), &QPushButton::clicked, [this]()
			{
				ShowAddComponentMenu(addComponentButton->rect().bottomLeft());
			});

			addComponentButton->setContextMenuPolicy(Qt::CustomContextMenu);
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(const bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(const bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetSections(std::vector<cp::editorui::InspectorSection> _sections) override
		{
			Clear();
			applyingSections = true;

			for (const cp::editorui::InspectorSection& section : _sections)
			{
				auto* sectionItem = new QTreeWidgetItem();
				sectionItem->setText(0, ToQString(section.title));
				sectionItem->setData(0, Qt::UserRole, ToQString(section.id));
				sectionItem->setData(0, Qt::UserRole + 1, section.removable);
				tree->addTopLevelItem(sectionItem);

				for (const cp::editorui::InspectorField& field : section.fields)
				{
					auto* fieldItem = new QTreeWidgetItem(sectionItem);
					fieldItem->setText(0, ToQString(field.label));
					fieldItem->setFlags(fieldItem->flags() & ~Qt::ItemIsEditable);
					AttachEditor(*fieldItem, section.id, field);
				}
			}
			tree->expandAll();
			applyingSections = false;
		}

		void SetFieldEditedHandler(FieldEditedHandler _handler) override
		{
			fieldEditedHandler = std::move(_handler);
		}

		void SetAddComponentMenuHandler(AddComponentMenuHandler _handler) override
		{
			addComponentMenuHandler = std::move(_handler);
		}

		void SetRemoveComponentHandler(RemoveComponentHandler _handler) override
		{
			removeComponentHandler = std::move(_handler);
		}

		void Clear() override
		{
			tree->clear();
		}

		[[nodiscard]] QWidget* GetQWidget() const override
		{
			return mainWidget.get();
		}

	private:
		void EmitFieldEdited(
			const std::string& _sectionId,
			const std::string& _fieldId,
			const cp::editorui::InspectorField::Value& _value) const
		{
			if (applyingSections || !fieldEditedHandler)
			{
				return;
			}

			fieldEditedHandler(_sectionId, _fieldId, _value);
		}

		void AttachEditor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			switch (_field.valueType)
			{
			case cp::editorui::InspectorField::ValueType::Bool:
				AttachBoolEditor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::Int:
				AttachIntEditor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::Float:
				AttachFloatEditor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::Vec3:
				AttachVec3Editor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::String:
			default:
				AttachStringEditor(_item, _sectionId, _field);
				break;
			}
		}

		void AttachBoolEditor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			auto* checkBox = new QCheckBox(tree.get());
			checkBox->setEnabled(!_field.readOnly);
			if (const bool* value = std::get_if<bool>(&_field.value))
			{
				const QSignalBlocker blocker(checkBox);
				checkBox->setChecked(*value);
			}
			tree->setItemWidget(&_item, 1, checkBox);
			QObject::connect(checkBox, &QCheckBox::toggled, tree.get(), [this, sectionId = _sectionId, fieldId = _field.id](const bool _checked)
			{
				EmitFieldEdited(sectionId, fieldId, _checked);
			});
		}

		void AttachIntEditor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			auto* spin = new QSpinBox(tree.get());
			spin->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
			spin->setEnabled(!_field.readOnly);
			if (const int64_t* value = std::get_if<int64_t>(&_field.value))
			{
				const int clamped = static_cast<int>(std::clamp<int64_t>(
					*value,
					static_cast<int64_t>(std::numeric_limits<int>::min()),
					static_cast<int64_t>(std::numeric_limits<int>::max())
				));
				const QSignalBlocker blocker(spin);
				spin->setValue(clamped);
			}
			tree->setItemWidget(&_item, 1, spin);
			QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged), tree.get(), [this, sectionId = _sectionId, fieldId = _field.id](const int _value)
			{
				EmitFieldEdited(sectionId, fieldId, static_cast<int64_t>(_value));
			});
		}

		void AttachFloatEditor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			auto* spin = new QDoubleSpinBox(tree.get());
			spin->setRange(-1000000.0, 1000000.0);
			spin->setDecimals(4);
			spin->setSingleStep(0.01);
			spin->setEnabled(!_field.readOnly);
			if (const double* value = std::get_if<double>(&_field.value))
			{
				const QSignalBlocker blocker(spin);
				spin->setValue(*value);
			}
			tree->setItemWidget(&_item, 1, spin);
			QObject::connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged), tree.get(), [this, sectionId = _sectionId, fieldId = _field.id](const double _value)
			{
				EmitFieldEdited(sectionId, fieldId, _value);
			});
		}

		void AttachStringEditor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			if (_field.inputType == cp::editorui::InspectorField::InputType::FilePath && !_field.readOnly)
			{
				auto* dropWidget = new cp::editorqt::QtFileDropWidget(tree.get());
				if (const std::string* value = std::get_if<std::string>(&_field.value))
				{
					dropWidget->SetFilePath(std::filesystem::path(*value));
				}
				dropWidget->SetFileDropCallback([this, sectionId = _sectionId, fieldId = _field.id](const std::filesystem::path& _path)
				{
					EmitFieldEdited(sectionId, fieldId, _path.string());
				});
				tree->setItemWidget(&_item, 1, dropWidget);
				return;
			}

			auto* edit = new QLineEdit(tree.get());
			edit->setEnabled(!_field.readOnly);
			if (const std::string* value = std::get_if<std::string>(&_field.value))
			{
				const QSignalBlocker blocker(edit);
				edit->setText(ToQString(*value));
			}
			tree->setItemWidget(&_item, 1, edit);
			QObject::connect(edit, &QLineEdit::editingFinished, tree.get(), [this, edit, sectionId = _sectionId, fieldId = _field.id]()
			{
				EmitFieldEdited(sectionId, fieldId, ToStdString(edit->text()));
			});
		}

		void AttachVec3Editor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			auto* container = new QWidget(tree.get());
			auto* layout = new QHBoxLayout(container);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(4);

			auto* xSpin = new QDoubleSpinBox(container);
			auto* ySpin = new QDoubleSpinBox(container);
			auto* zSpin = new QDoubleSpinBox(container);
			for (QDoubleSpinBox* spin : {xSpin, ySpin, zSpin})
			{
				spin->setRange(-1000000.0, 1000000.0);
				spin->setDecimals(4);
				spin->setSingleStep(0.01);
				spin->setEnabled(!_field.readOnly);
				layout->addWidget(spin);
			}

			if (const cp::editorui::InspectorField::Vec3* vec = std::get_if<cp::editorui::InspectorField::Vec3>(&_field.value))
			{
				const QSignalBlocker xBlocker(xSpin);
				const QSignalBlocker yBlocker(ySpin);
				const QSignalBlocker zBlocker(zSpin);
				xSpin->setValue(vec->x);
				ySpin->setValue(vec->y);
				zSpin->setValue(vec->z);
			}

			tree->setItemWidget(&_item, 1, container);
			auto emitVec = [this, xSpin, ySpin, zSpin, sectionId = _sectionId, fieldId = _field.id]()
			{
				cp::editorui::InspectorField::Vec3 vec{
					.x = xSpin->value(),
					.y = ySpin->value(),
					.z = zSpin->value()
				};
				EmitFieldEdited(sectionId, fieldId, vec);
			};
			QObject::connect(xSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), tree.get(), [emitVec](double) { emitVec(); });
			QObject::connect(ySpin, qOverload<double>(&QDoubleSpinBox::valueChanged), tree.get(), [emitVec](double) { emitVec(); });
			QObject::connect(zSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), tree.get(), [emitVec](double) { emitVec(); });
		}

		void ShowAddComponentMenu(const QPoint& _position)
		{
			if (!addComponentMenuHandler)
			{
				return;
			}

			const auto menu = addComponentMenuHandler();
			const auto qtMenu = std::dynamic_pointer_cast<QtContextMenu>(menu);
			if (!qtMenu)
			{
				return;
			}

			const QPoint globalPos = addComponentButton->mapToGlobal(_position);
			qtMenu->Exec(globalPos);
		}

		std::unique_ptr<QWidget> mainWidget;
		std::unique_ptr<QTreeWidget> tree;
		std::unique_ptr<QPushButton> addComponentButton;
		FieldEditedHandler fieldEditedHandler;
		AddComponentMenuHandler addComponentMenuHandler;
		RemoveComponentHandler removeComponentHandler;
		bool applyingSections = false;
	};
}


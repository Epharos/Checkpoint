#pragma once

#include "EditorQtCommon.hpp"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTreeWidget>

#include <limits>

namespace cp::editorqt
{
	class QtSceneConfigView final : public cp::editorui::ISceneConfigView, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtSceneConfigView(std::string _id)
			: QtWidgetBase(std::move(_id))
			, root(new QWidget())
		{
			auto* layout = new QVBoxLayout(root.get());
			layout->setContentsMargins(4, 4, 4, 4);
			layout->setSpacing(6);

			auto* passGroup  = new QGroupBox("Render Passes");
			auto* passLayout = new QVBoxLayout(passGroup);
			passLayout->setContentsMargins(4, 4, 4, 4);
			passList = new QListWidget();
			passList->setObjectName("sceneConfig.passList");
			passLayout->addWidget(passList);
			layout->addWidget(passGroup);

			auto* systemGroup  = new QGroupBox("ECS Systems");
			auto* systemLayout = new QVBoxLayout(systemGroup);
			systemLayout->setContentsMargins(4, 4, 4, 4);
			systemList = new QListWidget();
			systemList->setObjectName("sceneConfig.systemList");
			systemLayout->addWidget(systemList);
			layout->addWidget(systemGroup);

			// Parameters panel (hidden until an item is selected)
			paramsGroup = new QGroupBox("Parameters");
			paramsGroup->setObjectName("sceneConfig.paramsGroup");
			paramsGroup->setVisible(false);
			auto* paramsLayout = new QVBoxLayout(paramsGroup);
			paramsLayout->setContentsMargins(4, 4, 4, 4);
			paramsTree = new QTreeWidget();
			paramsTree->setObjectName("sceneConfig.paramsTree");
			paramsTree->setColumnCount(2);
			paramsTree->setHeaderLabels(QStringList{ "Property", "Value" });
			paramsTree->header()->setStretchLastSection(true);
			paramsLayout->addWidget(paramsTree);
			layout->addWidget(paramsGroup);

			applyButton = new QPushButton("Apply");
			applyButton->setObjectName("sceneConfig.applyButton");
			layout->addWidget(applyButton);
			layout->addStretch();

			QObject::connect(applyButton, &QPushButton::clicked, [this]()
			{
				if (!applyHandler)
					return;

				std::vector<std::string> enabledPasses;
				std::vector<cp::editorui::ISceneConfigView::PassOrderEntry> passOrders;
				for (int i = 0; i < passList->count(); ++i)
				{
					QListWidgetItem* item = passList->item(i);
					const QString key = item->data(Qt::UserRole).toString();

					if (QWidget* w = passList->itemWidget(item))
					{
						const bool checked = w->findChild<QCheckBox*>() && w->findChild<QCheckBox*>()->isChecked();
						const int32_t order = w->findChild<QSpinBox*>() ? static_cast<int32_t>(w->findChild<QSpinBox*>()->value()) : 0;

						if (checked)
						{
							enabledPasses.push_back(ToStdString(key));
						}

						passOrders.push_back({ ToStdString(key), order });
					}
					else
					{
						// Fallback: item without custom widget (shouldn't happen)
						if (item->checkState() == Qt::Checked)
						{
							enabledPasses.push_back(ToStdString(key));
						}

						passOrders.push_back({ ToStdString(key), 0 });
					}
				}

				std::vector<std::string> enabledSystems;
				for (int i = 0; i < systemList->count(); ++i)
				{
					QListWidgetItem* item = systemList->item(i);
					if (item->checkState() == Qt::Checked)
						enabledSystems.push_back(ToStdString(item->data(Qt::UserRole).toString()));
				}

				applyHandler(std::move(enabledPasses), std::move(enabledSystems), std::move(passOrders));
			});

			QObject::connect(passList, &QListWidget::currentItemChanged,
				[this](QListWidgetItem* current, QListWidgetItem*)
				{
					if (!current)
						return;
					currentKey = ToStdString(current->data(Qt::UserRole).toString());
					currentIsPass = true;
					if (selectionChangedHandler)
						selectionChangedHandler(currentKey, true);
				});

			QObject::connect(systemList, &QListWidget::currentItemChanged,
				[this](QListWidgetItem* current, QListWidgetItem*)
				{
					if (!current)
						return;
					currentKey = ToStdString(current->data(Qt::UserRole).toString());
					currentIsPass = false;
					if (selectionChangedHandler)
						selectionChangedHandler(currentKey, false);
				});
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetRenderPassEntries(std::vector<cp::editorui::ISceneConfigView::RegistryEntry> _entries) override
		{
			passList->clear();
			for (const auto& entry : _entries)
			{
				auto* item = new QListWidgetItem(passList);
				const QString key = entry.key.empty() ? ToQString(entry.name) : ToQString(entry.key);
				item->setData(Qt::UserRole, key);

				// [checkbox with name] [Order: spinbox]
				auto* container = new QWidget();
				auto* hLayout = new QHBoxLayout(container);
				hLayout->setContentsMargins(2, 1, 2, 1);
				hLayout->setSpacing(6);

				auto* checkBox = new QCheckBox(ToQString(entry.name), container);
				checkBox->setChecked(entry.enabled);
				hLayout->addWidget(checkBox, 1);

				auto* orderLabel = new QLabel("Order:", container);
				auto* spin = new QSpinBox(container);
				spin->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
				spin->setValue(entry.order);
				spin->setFixedWidth(75);
				spin->setToolTip("Execution order: passes with a lower value always run before passes with a higher value");
				hLayout->addWidget(orderLabel);
				hLayout->addWidget(spin);

				// When the checkbox or spin inside the widget is clicked, select the item
				QObject::connect(checkBox, &QCheckBox::clicked, passList, [this, item]()
				{
					passList->setCurrentItem(item);
				});

				item->setSizeHint(container->sizeHint());
				passList->setItemWidget(item, container);
			}
		}

		void SetSystemEntries(std::vector<cp::editorui::ISceneConfigView::RegistryEntry> _entries) override
		{
			systemList->clear();
			for (const auto& entry : _entries)
			{
				auto* item = new QListWidgetItem(ToQString(entry.name), systemList);
				item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
				item->setCheckState(entry.enabled ? Qt::Checked : Qt::Unchecked);
				const QString key = entry.key.empty() ? ToQString(entry.name) : ToQString(entry.key);
				item->setData(Qt::UserRole, key);
			}
		}

		void SetApplyHandler(ApplyHandler _handler) override
		{
			applyHandler = std::move(_handler);
		}

		void SetSelectionChangedHandler(SelectionChangedHandler _handler) override
		{
			selectionChangedHandler = std::move(_handler);
		}

		void SetParamFieldEditedHandler(ParamFieldEditedHandler _handler) override
		{
			paramFieldEditedHandler = std::move(_handler);
		}

		void SetParamSections(std::string_view _title, std::vector<cp::editorui::InspectorSection> _sections) override
		{
			paramsGroup->setTitle(QString("Parameters: %1").arg(ToQString(_title)));
			paramsTree->clear();

			applyingParamSections = true;
			for (const cp::editorui::InspectorSection& section : _sections)
			{
				auto* sectionItem = new QTreeWidgetItem();
				sectionItem->setText(0, ToQString(section.title));
				paramsTree->addTopLevelItem(sectionItem);

				for (const cp::editorui::InspectorField& field : section.fields)
				{
					auto* fieldItem = new QTreeWidgetItem(sectionItem);
					fieldItem->setText(0, ToQString(field.label));
					fieldItem->setFlags(fieldItem->flags() & ~Qt::ItemIsEditable);
					AttachParamEditor(*fieldItem, section.id, field);
				}
			}
			paramsTree->expandAll();
			applyingParamSections = false;

			paramsGroup->setVisible(true);
		}

		void ClearParamSections() override
		{
			paramsTree->clear();
			paramsGroup->setVisible(false);
		}

		[[nodiscard]] QWidget* GetQWidget() const override { return root.get(); }

	private:
		void EmitParamFieldEdited(
			const std::string& _sectionId,
			const std::string& _fieldId,
			const cp::editorui::InspectorField::Value& _value) const
		{
			if (applyingParamSections || !paramFieldEditedHandler)
				return;
			paramFieldEditedHandler(currentKey, currentIsPass, _sectionId, _fieldId, _value);
		}

		void AttachParamEditor(
			QTreeWidgetItem& _item,
			const std::string& _sectionId,
			const cp::editorui::InspectorField& _field)
		{
			switch (_field.valueType)
			{
			case cp::editorui::InspectorField::ValueType::Bool:
				AttachParamBoolEditor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::Int:
				AttachParamIntEditor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::Float:
				AttachParamFloatEditor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::Vec3:
				AttachParamVec3Editor(_item, _sectionId, _field);
				break;
			case cp::editorui::InspectorField::ValueType::String:
			default:
				AttachParamStringEditor(_item, _sectionId, _field);
				break;
			}
		}

		void AttachParamBoolEditor(QTreeWidgetItem& _item, const std::string& _sectionId, const cp::editorui::InspectorField& _field)
		{
			auto* checkBox = new QCheckBox(paramsTree);
			checkBox->setEnabled(!_field.readOnly);
			if (const bool* v = std::get_if<bool>(&_field.value))
			{
				const QSignalBlocker blocker(checkBox);
				checkBox->setChecked(*v);
			}
			paramsTree->setItemWidget(&_item, 1, checkBox);
			QObject::connect(checkBox, &QCheckBox::toggled, paramsTree, [this, sid = _sectionId, fid = _field.id](bool checked)
			{
				EmitParamFieldEdited(sid, fid, checked);
			});
		}

		void AttachParamIntEditor(QTreeWidgetItem& _item, const std::string& _sectionId, const cp::editorui::InspectorField& _field)
		{
			auto* spin = new QSpinBox(paramsTree);
			spin->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
			spin->setEnabled(!_field.readOnly);
			if (const int64_t* v = std::get_if<int64_t>(&_field.value))
			{
				const QSignalBlocker blocker(spin);
				spin->setValue(static_cast<int>(*v));
			}
			paramsTree->setItemWidget(&_item, 1, spin);
			QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged), paramsTree, [this, sid = _sectionId, fid = _field.id](int val)
			{
				EmitParamFieldEdited(sid, fid, static_cast<int64_t>(val));
			});
		}

		void AttachParamFloatEditor(QTreeWidgetItem& _item, const std::string& _sectionId, const cp::editorui::InspectorField& _field)
		{
			auto* spin = new QDoubleSpinBox(paramsTree);
			spin->setRange(-1000000.0, 1000000.0);
			spin->setDecimals(4);
			spin->setSingleStep(0.01);
			spin->setEnabled(!_field.readOnly);
			if (const double* v = std::get_if<double>(&_field.value))
			{
				const QSignalBlocker blocker(spin);
				spin->setValue(*v);
			}
			paramsTree->setItemWidget(&_item, 1, spin);
			QObject::connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged), paramsTree, [this, sid = _sectionId, fid = _field.id](double val)
			{
				EmitParamFieldEdited(sid, fid, val);
			});
		}

		void AttachParamStringEditor(QTreeWidgetItem& _item, const std::string& _sectionId, const cp::editorui::InspectorField& _field)
		{
			auto* edit = new QLineEdit(paramsTree);
			edit->setEnabled(!_field.readOnly);
			if (const std::string* v = std::get_if<std::string>(&_field.value))
			{
				const QSignalBlocker blocker(edit);
				edit->setText(ToQString(*v));
			}
			paramsTree->setItemWidget(&_item, 1, edit);
			QObject::connect(edit, &QLineEdit::editingFinished, paramsTree, [this, edit, sid = _sectionId, fid = _field.id]()
			{
				EmitParamFieldEdited(sid, fid, ToStdString(edit->text()));
			});
		}

		void AttachParamVec3Editor(QTreeWidgetItem& _item, const std::string& _sectionId, const cp::editorui::InspectorField& _field)
		{
			auto* container = new QWidget(paramsTree);
			auto* hLayout   = new QHBoxLayout(container);
			hLayout->setContentsMargins(0, 0, 0, 0);
			hLayout->setSpacing(4);

			auto* xSpin = new QDoubleSpinBox(container);
			auto* ySpin = new QDoubleSpinBox(container);
			auto* zSpin = new QDoubleSpinBox(container);
			for (QDoubleSpinBox* spin : {xSpin, ySpin, zSpin})
			{
				spin->setRange(-1000000.0, 1000000.0);
				spin->setDecimals(4);
				spin->setSingleStep(0.01);
				spin->setEnabled(!_field.readOnly);
				hLayout->addWidget(spin);
			}
			if (const cp::editorui::InspectorField::Vec3* vec = std::get_if<cp::editorui::InspectorField::Vec3>(&_field.value))
			{
				const QSignalBlocker xb(xSpin), yb(ySpin), zb(zSpin);
				xSpin->setValue(vec->x);
				ySpin->setValue(vec->y);
				zSpin->setValue(vec->z);
			}
			paramsTree->setItemWidget(&_item, 1, container);

			auto emitVec = [this, xSpin, ySpin, zSpin, sid = _sectionId, fid = _field.id]()
			{
				EmitParamFieldEdited(sid, fid, cp::editorui::InspectorField::Vec3{ xSpin->value(), ySpin->value(), zSpin->value() });
			};
			QObject::connect(xSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), paramsTree, [emitVec](double) { emitVec(); });
			QObject::connect(ySpin, qOverload<double>(&QDoubleSpinBox::valueChanged), paramsTree, [emitVec](double) { emitVec(); });
			QObject::connect(zSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), paramsTree, [emitVec](double) { emitVec(); });
		}

		std::unique_ptr<QWidget> root;
		QListWidget* passList = nullptr;
		QListWidget* systemList = nullptr;
		QGroupBox* paramsGroup = nullptr;
		QTreeWidget* paramsTree  = nullptr;
		QPushButton* applyButton = nullptr;

		ApplyHandler applyHandler;
		SelectionChangedHandler selectionChangedHandler;
		ParamFieldEditedHandler paramFieldEditedHandler;

		std::string currentKey;
		bool currentIsPass = false;
		bool applyingParamSections = false;
	};
}

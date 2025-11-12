#pragma once

#include "pch.hpp"
#include <QWidget>

namespace cp {
	class StringField : public QWidget {
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif

	public:
			StringField(std::string* _value, const std::string& _fieldName, QWidget* parent = nullptr)
				: QWidget(parent), valuePtr(_value)
			{
				widget = new QWidget(this);
				QHBoxLayout* layout = new QHBoxLayout(widget);
				label = new QLabel(QString::fromStdString(_fieldName), widget);
				lineEdit = new QLineEdit(QString::fromStdString(*valuePtr), widget);
				lineEdit->setStyleSheet("QLineEdit { padding: 2px 4px; background-color: #1A1F2B; }");
				layout->addWidget(label);
				layout->addWidget(lineEdit);
				widget->setLayout(layout);
				QObject::connect(lineEdit, &QLineEdit::textChanged, this, &StringField::OnTextChanged);
			}

			void OnTextChanged(const QString& _text) {
				*valuePtr = _text.toStdString();
			}

			virtual ~StringField() {
				delete widget;
			}
	
		protected:
			QWidget* widget = nullptr;
			QLabel* label = nullptr;
			QLineEdit* lineEdit = nullptr;
			std::string* valuePtr = nullptr;
	};
}
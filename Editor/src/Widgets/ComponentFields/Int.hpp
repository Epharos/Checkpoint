#pragma once

#include "../../pch.hpp"

class Int : public QWidget
{
	Q_OBJECT

protected:
	int* value = nullptr;
	QVBoxLayout* layout = nullptr;
public:
	Int(int* _string, const std::string& _fieldName, QWidget* parent = nullptr) : QWidget(parent), value(_string)
	{
		QLabel* fieldName = new QLabel(QString::fromStdString(_fieldName), this);
		QLineEdit* valueEdit = new QLineEdit(QString::fromStdString(std::to_string(*value)), this);

		layout = new QVBoxLayout(this);
		layout->addWidget(fieldName);
		layout->addWidget(valueEdit);

		connect(valueEdit, &QLineEdit::textChanged, this, &Int::UpdateValue);

		connect(this, &Int::ValueChanged, [=] {
			valueEdit->setText(QString::fromStdString(std::to_string(*value)));
			});
	}

signals:
	void ValueChanged();

private slots:
	void UpdateValue(const QString& _text)
	{
		*value = _text.toInt();
		emit ValueChanged();
	}
};
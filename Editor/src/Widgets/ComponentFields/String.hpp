#pragma once

#include "../../pch.hpp"

class String : public QWidget
{
	Q_OBJECT

protected:
	std::string* value = nullptr;

public:
	String(std::string* _string, const std::string& _fieldName, QWidget* parent = nullptr) : QWidget(parent), value(_string)
	{
		QLabel* fieldName = new QLabel(QString::fromStdString(_fieldName), this);
		QLineEdit* valueEdit = new QLineEdit(QString::fromStdString(*value), this);

		connect(valueEdit, &QLineEdit::textChanged, this, &String::UpdateValue);

		connect(this, &String::ValueChanged, [=] {
			valueEdit->setText(QString::fromStdString(*value));
			});
	}

signals:
	void ValueChanged();

private slots:
	void UpdateValue(const QString& _text)
	{
		*value = _text.toStdString();
		emit ValueChanged();
	}
};
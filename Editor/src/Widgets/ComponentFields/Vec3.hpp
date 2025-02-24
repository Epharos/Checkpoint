#pragma once

#include "../../pch.hpp"
#include "Field.hpp"


class Vec3 : public QWidget
{
	Q_OBJECT

protected:
	glm::vec3* value = nullptr;

	QBoxLayout* layout = nullptr;

public:
	Vec3(glm::vec3* _vec3, LayoutDirection _layout = Columns, QWidget* parent = nullptr) : QWidget(parent), value(_vec3)
	{
		switch (_layout)
		{
		case Lines:
			layout = new QVBoxLayout(this);
			break;
		case Columns:
			layout = new QHBoxLayout(this);
			break;
		}

		QLabel* xLabel = new QLabel("X", this);
		QLabel* yLabel = new QLabel("Y", this);
		QLabel* zLabel = new QLabel("Z", this);

		QLineEdit* xEdit = new QLineEdit(QString::number(value->x), this);
		QLineEdit* yEdit = new QLineEdit(QString::number(value->y), this);
		QLineEdit* zEdit = new QLineEdit(QString::number(value->z), this);

		QHBoxLayout* xLayout = new QHBoxLayout();
		QHBoxLayout* yLayout = new QHBoxLayout();
		QHBoxLayout* zLayout = new QHBoxLayout();

		xLayout->addWidget(xLabel);
		xLayout->addWidget(xEdit);
		yLayout->addWidget(yLabel);
		yLayout->addWidget(yEdit);
		zLayout->addWidget(zLabel);
		zLayout->addWidget(zEdit);

		layout->addLayout(xLayout);
		layout->addLayout(yLayout);
		layout->addLayout(zLayout);

		connect(xEdit, &QLineEdit::textChanged, this, &Vec3::UpdateX);
		connect(yEdit, &QLineEdit::textChanged, this, &Vec3::UpdateY);
		connect(zEdit, &QLineEdit::textChanged, this, &Vec3::UpdateZ);

		connect(this, &Vec3::ValueChanged, [=] {
			xEdit->setText(QString::number(value->x));
			yEdit->setText(QString::number(value->y));
			zEdit->setText(QString::number(value->z));
			});
	}

signals:
	void ValueChanged();

private slots:
	void UpdateX(const QString& _text)
	{
		value->x = _text.toFloat();
		emit ValueChanged();
	}

	void UpdateY(const QString& _text)
	{
		value->y = _text.toFloat();
		emit ValueChanged();
	}

	void UpdateZ(const QString& _text)
	{
		value->z = _text.toFloat();
		emit ValueChanged();
	}
};
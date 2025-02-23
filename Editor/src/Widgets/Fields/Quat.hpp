#pragma once

#include "../../pch.hpp"
#include "Field.hpp"

class Quat : public QWidget
{
	Q_OBJECT

protected:
	glm::quat* value = nullptr;

	QBoxLayout* layout = nullptr;

public:
	Quat(glm::quat* _quat, LayoutDirection _layout = Columns, QWidget* parent = nullptr) : QWidget(parent), value(_quat)
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
		QLabel* wLabel = new QLabel("W", this);

		QLineEdit* xEdit = new QLineEdit(QString::number(value->x), this);
		QLineEdit* yEdit = new QLineEdit(QString::number(value->y), this);
		QLineEdit* zEdit = new QLineEdit(QString::number(value->z), this);
		QLineEdit* wEdit = new QLineEdit(QString::number(value->w), this);

		QHBoxLayout* xLayout = new QHBoxLayout();
		QHBoxLayout* yLayout = new QHBoxLayout();
		QHBoxLayout* zLayout = new QHBoxLayout();
		QHBoxLayout* wLayout = new QHBoxLayout();

		xLayout->addWidget(xLabel);
		xLayout->addWidget(xEdit);
		yLayout->addWidget(yLabel);
		yLayout->addWidget(yEdit);
		zLayout->addWidget(zLabel);
		zLayout->addWidget(zEdit);
		wLayout->addWidget(wLabel);
		wLayout->addWidget(wEdit);

		layout->addLayout(xLayout);
		layout->addLayout(yLayout);
		layout->addLayout(zLayout);
		layout->addLayout(wLayout);

		connect(xEdit, &QLineEdit::textChanged, this, &Quat::UpdateX);
		connect(yEdit, &QLineEdit::textChanged, this, &Quat::UpdateY);
		connect(zEdit, &QLineEdit::textChanged, this, &Quat::UpdateZ);
		connect(wEdit, &QLineEdit::textChanged, this, &Quat::UpdateW);

		connect(this, &Quat::ValueChanged, [=] {
			xEdit->setText(QString::number(value->x));
			yEdit->setText(QString::number(value->y));
			zEdit->setText(QString::number(value->z));
			wEdit->setText(QString::number(value->w));
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

	void UpdateW(const QString& _text)
	{
		value->w = _text.toFloat();
		emit ValueChanged();
	}
};

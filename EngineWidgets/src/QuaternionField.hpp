#pragma once

#include "pch.hpp"
#include "ComponentField.hpp"

namespace cp
{
	class Quaternion : public ComponentField
	{
		Q_OBJECT

	protected:
		glm::quat* value = nullptr;
		QBoxLayout* layout = nullptr;

	public:
		//TODO : Add an identifier for the field (a name shown in the editor) like the Vectors do

		Quaternion(glm::quat* _quat, LayoutDirection _layout = Columns, QWidget* parent = nullptr)
			: ComponentField(parent), value(_quat)
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

			layout->addWidget(wLabel);
			layout->addWidget(wEdit);
			layout->addWidget(xLabel);
			layout->addWidget(xEdit);
			layout->addWidget(yLabel);
			layout->addWidget(yEdit);
			layout->addWidget(zLabel);
			layout->addWidget(zEdit);

			connect(xEdit, &QLineEdit::textChanged, this, &Quaternion::UpdateX);
			connect(yEdit, &QLineEdit::textChanged, this, &Quaternion::UpdateY);
			connect(zEdit, &QLineEdit::textChanged, this, &Quaternion::UpdateZ);
			connect(wEdit, &QLineEdit::textChanged, this, &Quaternion::UpdateW);

			connect(this, &Quaternion::ValueChanged, [=]() {
				xEdit->setText(QString::number(value->x));
				yEdit->setText(QString::number(value->y));
				zEdit->setText(QString::number(value->z));
				wEdit->setText(QString::number(value->w));
				});
		}

	private slots:
		void UpdateX(const QString& text)
		{
			bool ok;
			float valueX = text.toFloat(&ok);
			if (ok) value->x = valueX;
			emit ValueChanged();
		}

		void UpdateY(const QString& text)
		{
			bool ok;
			float valueY = text.toFloat(&ok);
			if (ok) value->y = valueY;
			emit ValueChanged();
		}

		void UpdateZ(const QString& text)
		{
			bool ok;
			float valueZ = text.toFloat(&ok);
			if (ok) value->z = valueZ;
			emit ValueChanged();
		}

		void UpdateW(const QString& text)
		{
			bool ok;
			float valueW = text.toFloat(&ok);
			if (ok) value->w = valueW;
			emit ValueChanged();
		}
	};
}
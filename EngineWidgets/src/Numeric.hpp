#pragma once

#include "pch.hpp"
#include "ComponentField.hpp"

namespace cp
{
	template<typename T>
	concept NumericType = std::is_integral_v<T>;

	template<NumericType N>
	class Numeric : public ComponentField
	{
	protected:
		N* value = nullptr;
		QVBoxLayout* layout = nullptr;
	public:
		Numeric(N* _val, const std::string& _fieldName, QWidget* parent = nullptr) : ComponentField(parent), value(_val)
		{
			QLabel* fieldName = new QLabel(QString::fromStdString(_fieldName), this);
			QLineEdit* valueEdit = new QLineEdit(QString::fromStdString(std::to_string(*value)), this);

			layout = new QVBoxLayout(this);
			layout->addWidget(fieldName);
			layout->addWidget(valueEdit);

			connect(valueEdit, &QLineEdit::textChanged, this, &Numeric::UpdateValue);

			connect(this, &Numeric::ValueChanged, [=] {
				valueEdit->setText(QString::fromStdString(std::to_string(*value)));
				});
		}

	private slots:
		void UpdateValue(const QString& _text)
		{
			int temp = _text.toInt();
			*value = static_cast<N>(temp);
			emit ValueChanged();
		}
	};
}
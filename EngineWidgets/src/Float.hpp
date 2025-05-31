#pragma once

#include "pch.hpp"
#include "ComponentField.hpp"

namespace cp
{
	template<typename T>
	concept FloatingType = std::is_floating_point_v<T>;

	template<FloatingType N>
	class FloatWidget : public ComponentField
	{
	protected:
		N* value = nullptr;
		QVBoxLayout* layout = nullptr;
	public:
		FloatWidget(N* _val, const std::string& _fieldName, QWidget* parent = nullptr) : ComponentField(parent), value(_val)
		{
			QLabel* fieldName = new QLabel(QString::fromStdString(_fieldName), this);
			QLineEdit* valueEdit = new QLineEdit(QString::number(*value), this);

			layout = new QVBoxLayout(this);
			layout->addWidget(fieldName);
			layout->addWidget(valueEdit);

			connect(valueEdit, &QLineEdit::textChanged, this, &FloatWidget::UpdateValue);

			connect(this, &FloatWidget::ValueChanged, [=] {
				valueEdit->setText(QString::number(*value));
				});
		}

	private slots:
		void UpdateValue(const QString& _text)
		{
			double temp = _text.toDouble();
			*value = static_cast<N>(temp);
			emit ValueChanged();
		}
	};
}
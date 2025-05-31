#pragma once

#include "pch.hpp"
#include "ComponentField.hpp"

namespace cp
{
	template<typename T>
	concept VectorType = std::_Is_any_of_v<T, 
		glm::vec2, glm::vec3, glm::vec4,
		glm::vec<2, glm::f32, glm::defaultp>, glm::vec<3, glm::f32, glm::defaultp>, glm::vec<4, glm::f32, glm::defaultp>>;

	template<VectorType V, int SIZE>
	class VectorWidget : public ComponentField
	{
	protected:
		V* value = nullptr;
		QVBoxLayout* layout = nullptr;
	public:
		VectorWidget(V* _val, const std::string& _fieldName, const LayoutDirection& _layoutDir = cp::LayoutDirection::Lines, QWidget* parent = nullptr) : ComponentField(parent), value(_val)
		{
			QLabel* fieldName = new QLabel(QString::fromStdString(_fieldName), this);

			if (SIZE < 2 || SIZE > 4)
			{
				throw std::runtime_error("VectorWidget only supports vectors of size 2, 3, or 4");
				return;
			}

			QLineEdit** valueEdits = new QLineEdit*[SIZE];

			layout = new QVBoxLayout(this);

			layout->addWidget(fieldName);

			QBoxLayout* valuesLayout = nullptr;

			if (_layoutDir == cp::LayoutDirection::Lines)
			{
				valuesLayout = new QVBoxLayout();
			}
			else
			{
				valuesLayout = new QHBoxLayout();
			}
			
			for (int i = 0; i < SIZE; ++i)
			{
				QHBoxLayout* valueLayout = new QHBoxLayout();
				QLabel* label = new QLabel(QString::fromStdString("Component " + std::to_string(i)), this);

				switch (i)
				{
				case 0:
					label->setText("X");
					break;
				case 1:
					label->setText("Y");
					break;
				case 2:
					label->setText("Z");
					break;
				case 3:
					label->setText("W");
					break;
				}

				valueLayout->addWidget(label);
				valueEdits[i] = new QLineEdit(QString::fromStdString(std::to_string((*value)[i])), this);
				valueLayout->addWidget(valueEdits[i]);
				valuesLayout->addLayout(valueLayout);
				connect(valueEdits[i], &QLineEdit::textChanged, this, [=] { UpdateValue(i, valueEdits[i]->text()); });
				connect(this, &VectorWidget::ValueChanged, [=] {
					valueEdits[i]->setText(QString::fromStdString(std::to_string((*value)[i])));
					});
			}

			layout->addLayout(valuesLayout);
		}
	private slots:
		void UpdateValue(const int& i, const QString& _text)
		{
			double _value = _text.toDouble();
			(*value)[i] = _value;
			emit ValueChanged();
		}
	};
}
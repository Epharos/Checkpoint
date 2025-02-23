#pragma once

#include "../pch.hpp"
#include "ComponentBase.hpp"

class ComponentWidgetBase : public QWidget
{
public:
	ComponentWidgetBase(QWidget* _parent = nullptr) : QWidget(_parent) {}
	virtual ~ComponentWidgetBase() = default;
	virtual void Initialize() = 0;
};

template<typename T>
class ComponentWidget : public ComponentWidgetBase
{
protected:
	T& component;
	QVBoxLayout* layout;
	QLabel* componentName;

public:
	ComponentWidget(T& _component, const std::string& _componentName, QWidget* _parent = nullptr) : ComponentWidgetBase(_parent), component(_component)
	{
		if (!std::is_base_of<IComponentBase, T>::value)
		{
			throw std::invalid_argument("T must be a ComponentBase");
		}

		layout = new QVBoxLayout(this);
		componentName = new QLabel(QString::fromStdString(_componentName), this);
		layout->addWidget(componentName);
		setLayout(layout);
		layout->setAlignment(Qt::AlignTop);
	}

	virtual void Initialize() = 0;

	T& GetComponent() { return component; }
};
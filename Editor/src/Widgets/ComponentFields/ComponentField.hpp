#pragma once

#include "../../pch.hpp"

class ComponentField : public QWidget
{
	Q_OBJECT
public:
	ComponentField(QWidget* parent = nullptr) : QWidget(parent)
	{
	}
	virtual ~ComponentField() = default;

signals:
	void ValueChanged();
};
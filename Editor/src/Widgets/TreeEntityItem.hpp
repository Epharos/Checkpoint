#pragma once

#include "pch.hpp"

class TreeEntityItem : public QTreeWidgetItem
{
protected:
	Entity entity;
public:
	TreeEntityItem(Entity _entity, QTreeWidget* _parent) : QTreeWidgetItem(_parent), entity(_entity)
	{
		setText(0, QString::fromStdString(_entity.GetDisplayName()));
	}

	Entity& GetEntity() { return entity; }
};
#pragma once

#include "../pch.hpp"

class TreeEntityItem : public QTreeWidgetItem
{
protected:
	cp::Entity entity;
public:
	TreeEntityItem(cp::Entity _entity, QTreeWidget* _parent) : QTreeWidgetItem(_parent), entity(_entity)
	{
		setText(0, QString::fromStdString(_entity.GetDisplayName()));
	}

	cp::Entity& GetEntity() { return entity; }
};
#pragma once

#include <QtWidgets/qtreewidget.h>
#include <QtCore/qstring.h>
#include "ECSWrapper.hpp"

namespace cp {
	class TreeEntityItem : public QTreeWidgetItem
	{
	protected:
		cp::EntityAsset entity;
	public:
		TreeEntityItem(cp::EntityAsset _entity, QTreeWidget* _parent) : QTreeWidgetItem(_parent), entity(_entity)
		{
			setText(0, QString::fromStdString(_entity.name.empty() ? "Entity" : _entity.name));
		}

		cp::EntityAsset& GetEntity() { return entity; }
	};

	class SceneHierarchy : public QWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	protected:
		cp::SceneAsset* sceneAsset;
		QLabel* sceneNameLabel;
		QTreeWidget* treeWidget;

		QVBoxLayout* globalLayout;
	public:
		SceneHierarchy(cp::SceneAsset* _sceneAsset = nullptr, QWidget* _parent = nullptr) {
			globalLayout = new QVBoxLayout(this);

			sceneNameLabel = new QLabel();
			sceneNameLabel->setText("No scene name");
			globalLayout->addWidget(sceneNameLabel);

			treeWidget = new QTreeWidget();
			treeWidget->setHeaderHidden(true);
			globalLayout->addWidget(treeWidget);

			InitTree(_sceneAsset);
		}

		void InitTree(cp::SceneAsset* _sceneAsset) {
			treeWidget->clear();

			sceneAsset = _sceneAsset;

			if (_sceneAsset) {
				sceneNameLabel->setText(QString::fromStdString(_sceneAsset->name.empty() ? "No scene name" : _sceneAsset->name));
				//treeWidget->setHeaderLabel(QString::fromStdString(_sceneAsset->name));

				for (auto entityAsset : _sceneAsset->entities) {
					TreeEntityItem* item = new TreeEntityItem(entityAsset, treeWidget);
				}
			}
		}
	};
}
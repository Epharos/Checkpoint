#pragma once

#include <QtWidgets/qtreewidget.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qstyleditemdelegate.h>
#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include "ECSWrapper.hpp"

#define SCENE_HIERARCHY_ITEM_MARGIN 4
#define SCENE_HIERARCHY_ITEM_ICON_SIZE 16
#define SCENE_HIERARCHY_ITEM_ICON_COUNT 3
#define SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING 2

namespace cp {
#pragma region SceneHierarchyItemDelegate
	class SceneHierarchyItemDelegate : public QStyledItemDelegate
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	public:
		explicit SceneHierarchyItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			painter->save();

			bool isSelected = option.state & QStyle::State_Selected;
			bool isHovered = option.state & QStyle::State_MouseOver;

			QColor bgColor = QColor("#B987FF");
			bgColor.setAlpha(isSelected ? 10 : (isHovered ? 5 : 0));

			QString entityName = index.data(Qt::DisplayRole).toString();
			QIcon entityIcon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

			const int sideAreaWidth = (SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING) * SCENE_HIERARCHY_ITEM_ICON_COUNT;

			QRect rect = option.rect;
			QRect leftRect{ rect.left(), rect.top(), sideAreaWidth, rect.height() };
			QRect splitterRect{ leftRect.right(), rect.top(), 1, rect.height() };
			QRect mainRect{ leftRect.right() + SCENE_HIERARCHY_ITEM_MARGIN, rect.top(), rect.width() - sideAreaWidth - SCENE_HIERARCHY_ITEM_MARGIN * 2, rect.height() };

			QColor leftAreaColor = QColor("#292A38");
			QColor splitterColor = QColor("#3E465A");

			painter->fillRect(leftRect, leftAreaColor);
			painter->fillRect(splitterRect, splitterColor);

			painter->fillRect(mainRect, bgColor);

			if (isHovered) {
				static QIcon lockIcon("Editor_Resources/Icons/lock.svg");
				static QIcon unlockIcon("Editor_Resources/Icons/unlock.svg");
				static QIcon visibilityIcon("Editor_Resources/Icons/visibility.svg");
				static QIcon invisibilityIcon("Editor_Resources/Icons/invisibility.svg");
				static QIcon starIcon("Editor_Resources/Icons/star.svg");

				int x = leftRect.left();
				const int y = leftRect.center().y() - SCENE_HIERARCHY_ITEM_ICON_SIZE / 2;

				lockIcon.paint(painter, QRect{ x, y, SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE });
				x += SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING;
				visibilityIcon.paint(painter, QRect{ x, y, SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE });
				x += SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING;
				painter->setPen(QColor("#D0D3DC"));
				starIcon.paint(painter, QRect{ x, y, SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE });
			}

			int iconY = mainRect.center().y() - SCENE_HIERARCHY_ITEM_ICON_SIZE / 2;
			entityIcon.paint(painter, QRect{ mainRect.left(), iconY, SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE });

			QRect textRect = mainRect.adjusted(SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_MARGIN, 0, 0, 0);
			QColor textColor = isSelected ? QColor("#A66BFF") : isHovered ? QColor("#B987FF") : QColor("#D0D3DC");

			painter->setPen(textColor);
			painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, entityName);

			painter->restore();
		}

		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			return QSize(option.rect.width(), 24);
		}
	};
#pragma endregion

#pragma region EntityTreeItem
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
#pragma endregion

#pragma region SceneTreeWidget
	class SceneTreeWidget : public QTreeWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif

	public:
		explicit SceneTreeWidget(QWidget* parent = nullptr) : QTreeWidget(parent)
		{
			setHeaderHidden(true);
			setItemDelegate(new SceneHierarchyItemDelegate(this));
			setSelectionMode(QAbstractItemView::ExtendedSelection);
			setMouseTracking(true);
			setRootIsDecorated(false);
			setIndentation(0);
			setMinimumWidth(200);

			setStyleSheet(R"(
				QTreeWidget
				{
					background-color: #1A1F2B;
					border: 1px solid #3E465A;
					border-radius: 2px;
					outline: none;
					padding: 0px;
					margin: 0px;
				}

				QTreeWidget::item
				{
					border: none;
					outline: none;
					background-color: transparent;
				}
			)");
		}

		void mousePressEvent(QMouseEvent* event) override
		{
			QModelIndex index = indexAt(event->pos());
			if (!index.isValid()) return;

			QRect rect = visualRect(index);
			QRect sideAreaRect{ rect.left() + SCENE_HIERARCHY_ITEM_MARGIN, rect.top(), 
				(SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING) * SCENE_HIERARCHY_ITEM_ICON_COUNT, rect.height() };

			if (sideAreaRect.contains(event->pos())) {
				int relativeX = event->pos().x() - sideAreaRect.left();
				int iconIndex = relativeX / (SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING);

				switch (iconIndex) {
					case 0: LOG_DEBUG("Lock icon clicked"); break;
					case 1: LOG_DEBUG("Visibility icon clicked"); break;
					case 2: LOG_DEBUG("Star icon clicked"); break;
				}

				return;
			}

			QTreeWidget::mousePressEvent(event);
		}
	};
#pragma endregion

#pragma region SceneHierarchy
	class SceneHierarchy : public QWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	protected:
		cp::SceneAsset* sceneAsset;
		QLabel* sceneNameLabel;
		SceneTreeWidget* treeWidget;

		QVBoxLayout* globalLayout;
	public:
		SceneHierarchy(cp::SceneAsset* _sceneAsset = nullptr, QWidget* _parent = nullptr) {
			globalLayout = new QVBoxLayout(this);

			sceneNameLabel = new QLabel();
			sceneNameLabel->setText("No scene name");
			globalLayout->addWidget(sceneNameLabel);

			treeWidget = new SceneTreeWidget();
			//treeWidget->setHeaderHidden(true);
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
#pragma endregion
}
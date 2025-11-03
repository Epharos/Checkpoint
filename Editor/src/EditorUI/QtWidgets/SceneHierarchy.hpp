#pragma once

#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtCore/qstring.h>
#include <QtWidgets/qstyleditemdelegate.h>
#include <QtGui/qevent.h>
#include "../../ECSWrapper.hpp"
#include "Helper.hpp"
#include "ColoredIconButton.hpp"
#include <format>

#define SCENE_HIERARCHY_ITEM_MARGIN 8
#define SCENE_HIERARCHY_ITEM_ICON_SIZE 14
#define SCENE_HIERARCHY_ITEM_ICON_COUNT 3
#define SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING 2

#ifdef USE_QT
Q_DECLARE_METATYPE(cp::EntityAsset*)
#endif

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
			static QPixmap lockIcon = SvgToPixmap("Editor_Resources/Icons/lock.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), QColor("#D0D3DC"));
			static QPixmap unlockIcon = SvgToPixmap("Editor_Resources/Icons/unlock.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), QColor("#D0D3DC"));
			static QPixmap visibilityIcon = SvgToPixmap("Editor_Resources/Icons/visibility.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), QColor("#D0D3DC"));
			static QPixmap invisibilityIcon = SvgToPixmap("Editor_Resources/Icons/invisibility.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), QColor("#D0D3DC"));
			static QPixmap starIcon = SvgToPixmap("Editor_Resources/Icons/star.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), QColor("#D0D3DC"));
			static QPixmap activeStarIcon = SvgToPixmap("Editor_Resources/Icons/star.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), QColor("#EADD4E"));

			painter->save();

			bool isSelected = option.state & QStyle::State_Selected;
			bool isHovered = option.state & QStyle::State_MouseOver;

			QColor bgColor = QColor("#B987FF");
			bgColor.setAlpha(isSelected ? 10 : 0);

			QString entityName = index.data(Qt::DisplayRole).toString();
			QIcon entityIcon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
			QVariant entityVariant = index.data(Qt::UserRole);
			const cp::EntityAsset* entityAsset = entityVariant.value<const cp::EntityAsset*>();

			if (!entityAsset)
			{
				painter->restore();
				LOG_WARNING("Invalid entity asset pointer");
				return;
			}

			const int iconAreaWidth = (SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING) * SCENE_HIERARCHY_ITEM_ICON_COUNT;

			QRect rect = option.rect;

			painter->fillRect(rect, bgColor);

			if (isHovered) {
				int x = rect.right() - iconAreaWidth - SCENE_HIERARCHY_ITEM_MARGIN;
				const int y = rect.center().y() - SCENE_HIERARCHY_ITEM_ICON_SIZE / 2;

				entityAsset->locked ? painter->drawPixmap(x, y, lockIcon) : painter->drawPixmap(x, y, unlockIcon);
				x += SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING;
				entityAsset->visible ? painter->drawPixmap(x, y, visibilityIcon) : painter->drawPixmap(x, y, invisibilityIcon);
				x += SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING;
				entityAsset->favorite ? painter->drawPixmap(x, y, activeStarIcon) : painter->drawPixmap(x, y, starIcon);
			}
			else
			{
				int x = rect.right() - iconAreaWidth - SCENE_HIERARCHY_ITEM_MARGIN;
				const int y = rect.center().y() - SCENE_HIERARCHY_ITEM_ICON_SIZE / 2;

				entityAsset->locked ? painter->drawPixmap(x, y, lockIcon) : void();
				x += SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING;
				!entityAsset->visible ? painter->drawPixmap(x, y, invisibilityIcon) : void();
				x += SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING;
				entityAsset->favorite ? painter->drawPixmap(x, y, activeStarIcon) : void();
			}

			QPixmap entityPixmap = SvgToPixmap("Editor_Resources/Icons/EntityIcons/default.svg", QSize(SCENE_HIERARCHY_ITEM_ICON_SIZE, SCENE_HIERARCHY_ITEM_ICON_SIZE), isSelected ? QColor("#A66BFF") : isHovered ? QColor("#B987FF") : QColor("#D0D3DC"));
			int iconY = rect.center().y() - SCENE_HIERARCHY_ITEM_ICON_SIZE / 2;
			painter->drawPixmap(rect.left() + SCENE_HIERARCHY_ITEM_MARGIN, iconY, entityPixmap);

			QRect textRect = rect.adjusted(SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_MARGIN + 8, 0, 0, 0);
			QColor textColor = isSelected ? QColor("#A66BFF") : isHovered ? QColor("#B987FF") : QColor("#D0D3DC");

			painter->setPen(textColor);
			QFont font = painter->font();
			font.setPointSize(10);
			font.setWeight(QFont::Weight::Medium);
			painter->setFont(font);
			painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, entityName);

			painter->restore();
		}

		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			return QSize(option.rect.width(), 20);
		}
	};
#pragma endregion

#pragma region EntityTreeItem
	class TreeEntityItem : public QTreeWidgetItem
	{
	protected:
		cp::EntityAsset* entity;
	public:
		TreeEntityItem(cp::EntityAsset* _entity, QTreeWidget* _parent) : QTreeWidgetItem(_parent), entity(_entity)
		{
			setText(0, QString::fromStdString(_entity->name.empty() ? "Entity" : _entity->name));
			setData(0, Qt::DecorationRole, QVariant::fromValue(QIcon("Editor_Resources/Icons/EntityIcons/default.svg")));
			setData(0, Qt::UserRole, QVariant::fromValue(entity));
		}

		cp::EntityAsset*& GetEntity() { return entity; }
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
					padding: 8px 0px;
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
			QRect sideAreaRect{ rect.right() - (SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING) * SCENE_HIERARCHY_ITEM_ICON_COUNT - SCENE_HIERARCHY_ITEM_MARGIN, rect.top(),
				rect.right() - SCENE_HIERARCHY_ITEM_MARGIN, rect.height()};

			if (sideAreaRect.contains(event->pos())) {
				int relativeX = event->pos().x() - sideAreaRect.left();
				int iconIndex = relativeX / (SCENE_HIERARCHY_ITEM_ICON_SIZE + SCENE_HIERARCHY_ITEM_SIDE_ICON_SPACING);

				QVariant entityVariant = index.data(Qt::UserRole);
				cp::EntityAsset* entityAsset = entityVariant.value<cp::EntityAsset*>();

				switch (iconIndex) {
				case 0: entityAsset->locked = !entityAsset->locked; break;
				case 1: entityAsset->visible = !entityAsset->visible; break;
				case 2: entityAsset->favorite = !entityAsset->favorite; break;
				}

				viewport()->update(rect);

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
	signals:
		void SceneUpdated(const SceneAsset* _scene);
		void EntitySelected(cp::EntityAsset* _entity);

	protected:
		cp::SceneAsset* sceneAsset;
		QPushButton* addEntityButton;
		QLineEdit* searchBar;
		ColoredIconButton* filterButton;
		ColoredIconButton* refreshButton;
		SceneTreeWidget* treeWidget;

		QVBoxLayout* globalLayout;
	public:
		SceneHierarchy(cp::SceneAsset* _sceneAsset = nullptr, QWidget* _parent = nullptr) {
			globalLayout = new QVBoxLayout(this);
			globalLayout->setContentsMargins(0, 8, 0, 0);
			globalLayout->setSpacing(0);

			setLayout(globalLayout);

			QHBoxLayout* actionsLayout = new QHBoxLayout();
			actionsLayout->setContentsMargins(8, 0, 8, 4);
			actionsLayout->setSpacing(8);

			addEntityButton = new QPushButton("+ Add entity");
			addEntityButton->setCursor(Qt::PointingHandCursor);
			addEntityButton->setFlat(true);
			addEntityButton->setStyleSheet("QPushButton { border: none; background: transparent; color: #D0D3DC; font-weight: bold; padding: 4px 0; }"
				"QPushButton:hover { color: #A66BFF; }");
			actionsLayout->addWidget(addEntityButton);

			searchBar = new QLineEdit();
			searchBar->setPlaceholderText("Search...");
			searchBar->setStyleSheet("QLineEdit { background: #23283A; color: #D0D3DC; border: 1px solid #3E465A; border-radius: 2px; padding: 2px 8px; }");
			
			QPixmap searchIcon = SvgToPixmap("Editor_Resources/Icons/search.svg", QSize(16, 16), QColor("#D0D3DC"));
			QAction* searchAction = new QAction(QIcon(searchIcon), "", searchBar);
			searchBar->addAction(searchAction, QLineEdit::LeadingPosition);
			
			actionsLayout->addWidget(searchBar, 1);

			filterButton = new ColoredIconButton("Editor_Resources/Icons/filter.svg", QSize(18, 18), QColor("#D0D3DC"), this);
			filterButton->setHoveredColor(QColor("#B987FF"));
			filterButton->setDisabledColor(QColor("#5A5F6E"));
			filterButton->setEnabled(false);
			actionsLayout->addWidget(filterButton);

			refreshButton = new ColoredIconButton("Editor_Resources/Icons/refresh.svg", QSize(18, 18), QColor("#D0D3DC"), this);
			refreshButton->setHoveredColor(QColor("#B987FF"));
			refreshButton->setDisabledColor(QColor("#5A5F6E"));
			actionsLayout->addWidget(refreshButton);

			actionsLayout->addStretch();

			globalLayout->addLayout(actionsLayout);

			treeWidget = new SceneTreeWidget();
			globalLayout->addWidget(treeWidget);

			InitTree(_sceneAsset);

			connect(addEntityButton, &QPushButton::clicked, this, [&]() {
				if (!sceneAsset) return;

				AddEntityToTree(); // For some obscure reason, creating the EntityAsset* or TreeEntityItem* directly causes a compile error (C1001) on MSVC

				//TODO: Update other systems about the new entity
			});

			connect(refreshButton, &QPushButton::clicked, this, [&]() {
				InitTree(sceneAsset);
			});

			connect(treeWidget, &QTreeWidget::itemClicked, this, [&](QTreeWidgetItem* item, int column) {
				QVariant entityVariant = item->data(0, Qt::UserRole);
				cp::EntityAsset* entityAsset = entityVariant.value<cp::EntityAsset*>();

				if (entityAsset) {
					emit EntitySelected(entityAsset);
				}
			});
		}

		void InitTree(cp::SceneAsset* _sceneAsset) {
			treeWidget->clear();

			sceneAsset = _sceneAsset;

			if (_sceneAsset) {
				emit SceneUpdated(_sceneAsset);

				for (auto& entityAsset : _sceneAsset->entities) {
					TreeEntityItem* item = new TreeEntityItem(entityAsset, treeWidget);
				}
			}
		}

	private:
		void AddEntityToTree() {
			cp::EntityAsset* newEntity = new cp::EntityAsset();
			newEntity->name = "New Entity";
			sceneAsset->entities.push_back(newEntity);
			auto& entityRef = sceneAsset->entities.back();

			TreeEntityItem* item = new TreeEntityItem(entityRef, treeWidget);
			treeWidget->scrollToItem(item);
		}
	};
#pragma endregion
}
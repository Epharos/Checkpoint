#pragma once

#include "EditorQtCommon.hpp"
#include "QtContextMenu.hpp"

namespace cp::editorqt
{
	class QtSceneHierarchyView final : public cp::editorui::ISceneHierarchyView, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtSceneHierarchyView(std::string _id)
			: QtWidgetBase(std::move(_id)),
			  tree(new QTreeWidget())
		{
			tree->setObjectName(ToQString(GetId()));
			tree->setHeaderHidden(true);
			tree->setContextMenuPolicy(Qt::CustomContextMenu);

			QObject::connect(tree.get(), &QTreeWidget::itemActivated, [this](QTreeWidgetItem* _item, int)
			{
				if (_item == nullptr || entityActivatedHandler == nullptr)
				{
					return;
				}
				entityActivatedHandler(_item->data(0, Qt::UserRole).toULongLong());
			});

			QObject::connect(tree.get(), &QTreeWidget::itemSelectionChanged, [this]()
			{
				const QList<QTreeWidgetItem*> selected = tree->selectedItems();
				if (selected.isEmpty())
				{
					selectedEntity.reset();
					return;
				}
				selectedEntity = selected.first()->data(0, Qt::UserRole).toULongLong();
			});

			QObject::connect(tree.get(), &QWidget::customContextMenuRequested, [this](const QPoint& _position)
			{
				if (!contextMenuHandler)
				{
					return;
				}

				QTreeWidgetItem* item = tree->itemAt(_position);
				if (item == nullptr)
				{
					return;
				}

				const auto menu = contextMenuHandler(item->data(0, Qt::UserRole).toULongLong());
				const auto qtMenu = std::dynamic_pointer_cast<QtContextMenu>(menu);
				if (!qtMenu)
				{
					return;
				}

				qtMenu->Exec(tree->viewport()->mapToGlobal(_position));
			});
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(const bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(const bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetNodes(std::vector<cp::editorui::SceneHierarchyNode> _nodes) override
		{
			tree->clear();
			for (const cp::editorui::SceneHierarchyNode& node : _nodes)
			{
				AddNode(nullptr, node);
			}
		}

		void SetSelectedEntity(std::optional<cp::editorui::EntityId> _entityId) override
		{
			selectedEntity = _entityId;
			if (!_entityId.has_value())
			{
				tree->clearSelection();
				return;
			}

			const QList<QTreeWidgetItem*> allItems = tree->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive);
			for (QTreeWidgetItem* item : allItems)
			{
				if (item->data(0, Qt::UserRole).toULongLong() == _entityId.value())
				{
					tree->setCurrentItem(item);
					item->setSelected(true);
					return;
				}
			}
		}

		[[nodiscard]] std::optional<cp::editorui::EntityId> GetSelectedEntity() const override
		{
			return selectedEntity;
		}

		void SetEntityActivatedHandler(EntityHandler _handler) override
		{
			entityActivatedHandler = std::move(_handler);
		}

		void SetContextMenuHandler(ContextMenuHandler _handler) override
		{
			contextMenuHandler = std::move(_handler);
		}

		[[nodiscard]] QWidget* GetQWidget() const override
		{
			return tree.get();
		}

	private:
		void AddNode(QTreeWidgetItem* _parent, const cp::editorui::SceneHierarchyNode& _node) const
		{
			auto* item = new QTreeWidgetItem();
			item->setText(0, ToQString(_node.label));
			item->setData(0, Qt::UserRole, static_cast<qulonglong>(_node.entityId));
			item->setExpanded(_node.expanded);

			if (_parent != nullptr)
			{
				_parent->addChild(item);
			}
			else
			{
				tree->addTopLevelItem(item);
			}

			for (const auto& child : _node.children)
			{
				AddNode(item, child);
			}
		}

		std::unique_ptr<QTreeWidget> tree;
		std::optional<cp::editorui::EntityId> selectedEntity;
		EntityHandler entityActivatedHandler;
		ContextMenuHandler contextMenuHandler;
	};
}


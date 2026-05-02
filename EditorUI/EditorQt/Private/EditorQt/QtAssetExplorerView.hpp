#pragma once

#include "EditorQtCommon.hpp"
#include "QtContextMenu.hpp"

namespace cp::editorqt
{
	class QtAssetExplorerView final : public cp::editorui::IAssetExplorerView, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtAssetExplorerView(std::string _id)
			: QtWidgetBase(std::move(_id)),
			  tree(new QTreeWidget())
		{
			tree->setObjectName(ToQString(GetId()));
			tree->setHeaderLabel("Assets");
			tree->setContextMenuPolicy(Qt::CustomContextMenu);

			QObject::connect(tree.get(), &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem* _item, int)
			{
				if (_item == nullptr || assetOpenedHandler == nullptr)
				{
					return;
				}
				assetOpenedHandler(std::filesystem::path(_item->data(0, Qt::UserRole).toString().toStdString()));
			});

			QObject::connect(tree.get(), &QWidget::customContextMenuRequested, [this](const QPoint& _position)
			{
				if (!contextMenuHandler)
				{
					return;
				}

				QTreeWidgetItem* item = tree->itemAt(_position);
				const std::filesystem::path path = item == nullptr
					                                   ? projectRoot
					                                   : std::filesystem::path(item->data(0, Qt::UserRole).toString().toStdString());
				const auto menu = contextMenuHandler(path);
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

		void SetProjectRoot(std::filesystem::path _root) override
		{
			projectRoot = std::move(_root);
			if (entries.empty())
			{
				RefreshEntriesFromDisk();
			}
			RebuildTree();
		}

		void SetEntries(std::vector<cp::editorui::AssetEntry> _entries) override
		{
			entries = std::move(_entries);
			RebuildTree();
		}

		[[nodiscard]] std::optional<std::filesystem::path> GetSelectedPath() const override
		{
			const QList<QTreeWidgetItem*> selected = tree->selectedItems();
			if (selected.isEmpty())
			{
				return std::nullopt;
			}
			return std::filesystem::path(selected.first()->data(0, Qt::UserRole).toString().toStdString());
		}

		void SetAssetOpenedHandler(AssetHandler _handler) override
		{
			assetOpenedHandler = std::move(_handler);
		}

		void SetContextMenuHandler(AssetContextMenuHandler _handler) override
		{
			contextMenuHandler = std::move(_handler);
		}

		[[nodiscard]] QWidget* GetQWidget() const override
		{
			return tree.get();
		}

	private:
		void RefreshEntriesFromDisk()
		{
			entries.clear();
			if (projectRoot.empty() || !std::filesystem::exists(projectRoot))
			{
				return;
			}

			for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(projectRoot))
			{
				entries.push_back({
					.path = dirEntry.path(),
					.isDirectory = dirEntry.is_directory()
				});
			}
		}

		static QTreeWidgetItem* EnsureChild(QTreeWidgetItem* _parent, const QString& _segment)
		{
			for (int index = 0; index < _parent->childCount(); ++index)
			{
				if (_parent->child(index)->text(0) == _segment)
				{
					return _parent->child(index);
				}
			}

			auto* item = new QTreeWidgetItem();
			item->setText(0, _segment);
			_parent->addChild(item);
			return item;
		}

		void RebuildTree() const
		{
			tree->clear();
			if (projectRoot.empty())
			{
				return;
			}

			auto* rootItem = new QTreeWidgetItem();
			rootItem->setText(0, ToQString(projectRoot.filename().string().empty() ? projectRoot.string() : projectRoot.filename().string()));
			rootItem->setData(0, Qt::UserRole, ToQString(projectRoot.string()));
			rootItem->setExpanded(true);
			tree->addTopLevelItem(rootItem);

			for (const cp::editorui::AssetEntry& entry : entries)
			{
				const std::filesystem::path path = entry.path.is_absolute() ? entry.path : (projectRoot / entry.path);
				std::filesystem::path relativePath = path.lexically_relative(projectRoot);
				if (relativePath.empty() || relativePath == ".")
				{
					continue;
				}

				QTreeWidgetItem* parent = rootItem;
				std::filesystem::path cumulative = projectRoot;
				for (const auto& segment : relativePath)
				{
					const QString qSegment = ToQString(segment.string());
					parent = EnsureChild(parent, qSegment);
					cumulative /= segment;
					parent->setData(0, Qt::UserRole, ToQString(cumulative.string()));
				}
			}
		}

		std::unique_ptr<QTreeWidget> tree;
		std::filesystem::path projectRoot;
		std::vector<cp::editorui::AssetEntry> entries;
		AssetHandler assetOpenedHandler;
		AssetContextMenuHandler contextMenuHandler;
	};
}


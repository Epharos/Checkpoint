#pragma once

#include <QWidget>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QRegularExpression>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include <functional>

namespace cp {
	struct Project;

	class FilterProxyModel : public QSortFilterProxyModel
	{
	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
		{
			QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
			return sourceModel()->data(index).toString().contains(filterRegularExpression());
		}
	};

	class ProjectListItemDelegate : public QStyledItemDelegate
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	public:
		explicit ProjectListItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			painter->save();
			bool isSelected = option.state & QStyle::State_Selected;
			bool isHovered = option.state & QStyle::State_MouseOver;
			QColor bgColor = QColor("#B987FF");
			bgColor.setAlpha(isSelected ? 10 : 0);
			QString text = index.data(Qt::DisplayRole).toString();
			// Draw background
			painter->fillRect(option.rect, bgColor);
			// Draw text
			QPen textPen(QColor("#D0D3DC"));
			painter->setPen(textPen);
			painter->drawText(option.rect.adjusted(8, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
			painter->restore();
		}
	};


	class ProjectListTreeWidget : public QTableView
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
		public:
			explicit ProjectListTreeWidget(QWidget* _parent = nullptr) : QTableView(_parent)
			{
				setStyleSheet(R"(
					QTableView
					{
						background-color: #1A1F2B;
						border-radius: 2px;
						outline: none;
						padding: 8px 0px;
						margin: 0px;
					}
					QTableView::item
					{
						border: none;
						outline: none;
						background-color: transparent;
					}
				)");
				setItemDelegate(new ProjectListItemDelegate(this));
			}
	};


	class ProjectList : public QWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	protected:
		ProjectListTreeWidget* projectListWidget;
		QStandardItemModel* model;
		FilterProxyModel* proxyModel;

		QLineEdit* searchBox;

	public:
		ProjectList(QWidget* _parent = nullptr);
		void PopulateProjectList(const std::vector<cp::Project>& _projects);
		void AddProject(const size_t _index, const cp::Project& _project);

		void AddProjectFocusedListener(std::function<void(const std::string&)> _callback);
		void AddProjectOpenedListener(std::function<void(const std::string&)> _callback);

	signals:
		void ProjectFocused(const std::string& _projectPath);
		void ProjectOpened(const std::string& _projectPath);

	private slots:
		void SelectProject(const QModelIndex& _index);
		void OpenProject(const QModelIndex& _index);
	};
}
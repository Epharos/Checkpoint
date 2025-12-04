#pragma once

#include <QWidget>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QRegularExpression>
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

	class ProjectListTreeWidget : public QTableView
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
		public:
			explicit ProjectListTreeWidget(QWidget* _parent = nullptr) : QTableView(_parent)
			{
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
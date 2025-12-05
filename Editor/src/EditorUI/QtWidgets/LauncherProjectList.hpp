#pragma once

#include "../../pch.hpp"

#include "../../CheckpointEditor.hpp"

#include <vector>
#include <functional>

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

#ifdef USE_QT
Q_DECLARE_METATYPE(cp::Project*)
#endif

class QAbstractListModel;
class QListView;
class QLineEdit;
class QStandardItemModel;

namespace cp {
	struct Project;

	class FilterProxyModel : public QSortFilterProxyModel
	{
	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
	};

	class ProjectListModel : public QAbstractListModel
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif

	public:
		explicit ProjectListModel(QObject* parent = nullptr);

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		QVariant data(const QModelIndex& index, int role) const override;

		void setProjects(const std::vector<Project>& projects);
		const Project& getProject(int row) const;

	private:
		std::vector<Project> m_projects;
	};

	class ProjectListItemDelegate : public QStyledItemDelegate
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	public:
		explicit ProjectListItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	};

	class ProjectListTreeWidget : public QListView
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
		public:
			explicit ProjectListTreeWidget(QWidget* _parent = nullptr);
	};

	class ProjectList : public QWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	protected:
		ProjectListTreeWidget* projectListView;
		ProjectListModel* projectModel;
		FilterProxyModel* proxyModel;

		QLineEdit* searchBox;

	public:
		ProjectList(QWidget* _parent = nullptr);
		void PopulateProjectList(const std::vector<cp::Project>& _projects);
		void AddProject(const cp::Project& _project);

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
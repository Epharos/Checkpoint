#pragma once

#include "../../pch.hpp"

#include <QtCore/qsortfilterproxymodel.h>
#include <QtWidgets/qheaderview.h>

class FilterProxyModel : public QSortFilterProxyModel
{
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
	{
		QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
		return sourceModel()->data(index).toString().contains(filterRegularExpression());
	}
};

class ProjectTableView : public QWidget
{
	Q_OBJECT

protected:
	QTableView* tableView;
	QStandardItemModel* model;
	FilterProxyModel* proxyModel;

	QLineEdit* searchBox;

public:
	ProjectTableView(QWidget* parent = nullptr) : QWidget(parent), model(nullptr), proxyModel(nullptr)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		searchBox = new QLineEdit(this);
		tableView = new QTableView(this);
		model = new QStandardItemModel(0, 4, this);
		proxyModel = new FilterProxyModel();

		layout->addWidget(searchBox);
		layout->addWidget(tableView);

		searchBox->setPlaceholderText("Search...");

		connect(searchBox, &QLineEdit::textChanged, this, &ProjectTableView::Search);
		connect(tableView, &QTableView::clicked, this, &ProjectTableView::SelectItem);
		connect(tableView, &QTableView::doubleClicked, this, &ProjectTableView::OpenProject);

		setMinimumWidth(700);
	}

	void Populate(const std::vector<cp::ProjectData>& _items)
	{
		if (model) delete model;
		if (proxyModel) delete proxyModel;

		model = new QStandardItemModel(_items.size(), 3, this);
		proxyModel = new FilterProxyModel();

		proxyModel->setSourceModel(model);
		model->setHorizontalHeaderLabels({ "Name", "Last Opened", "Version" });

		tableView->setModel(proxyModel);

		tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
		tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
		tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
		tableView->verticalHeader()->setVisible(false);

		tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
		tableView->setSelectionMode(QAbstractItemView::SingleSelection);
		tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

		tableView->setColumnWidth(1, 140);
		tableView->setColumnWidth(2, 80);

		for (int i = 0; i < _items.size(); i++)
		{
			const cp::ProjectData& data = _items[i];

			QStandardItem* nameItem = new QStandardItem(data.name);
			nameItem->setToolTip(data.path);
			QStandardItem* lastOpenedItem = new QStandardItem(data.lastOpened.toString("yyyy.MM.dd HH:mm"));
			QStandardItem* versionItem = new QStandardItem(QString::fromStdString(cp::VulkanContext::VersionToString(data.engineVersion)));

			model->setItem(i, 0, nameItem);
			model->setItem(i, 1, lastOpenedItem);
			model->setItem(i, 2, versionItem);
		}
	}

signals:
	void ItemSelected(std::string _path);
	void ProjectOpened(std::string _path);

private slots:
	void Search(const QString& _text)
	{
		proxyModel->setFilterRegularExpression(QRegularExpression(_text, QRegularExpression::CaseInsensitiveOption));
	}

	void SelectItem(const QModelIndex& _index)
	{
		QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(dynamic_cast<FilterProxyModel*>(tableView->model())->sourceModel());
		QStandardItem* item = model->item(_index.row(), 0);

		emit ItemSelected(item->toolTip().toStdString());
	}

	void OpenProject(const QModelIndex& _index)
	{
		QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(dynamic_cast<FilterProxyModel*>(tableView->model())->sourceModel());
		QStandardItem* item = model->item(_index.row(), 0);

		emit ProjectOpened(item->toolTip().toStdString());
	}
};
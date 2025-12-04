#include "LauncherProjectList.hpp"

#include "CheckpointEditor.hpp"
#include "EditorUI/QtWidgets/Helper.hpp"

cp::ProjectList::ProjectList(QWidget* parent) : QWidget(parent)
{
	projectListWidget = new ProjectListTreeWidget(this);
	model = new QStandardItemModel(0, 4, this);
	proxyModel = new cp::FilterProxyModel();

	searchBox = new QLineEdit(this);
	searchBox->setPlaceholderText("Search Projects...");
	searchBox->setStyleSheet(R"(
		QLineEdit {
			background-color: #1A1F2B;
			border: 1px solid #3E465A;
			border-radius: 2px;
			padding: 4px 8px;
			color: #D0D3DC;
		}
	)");

	QPixmap searchIcon = SvgToPixmap("Editor_Resources/Icons/search.svg", QSize(16, 16), QColor("#D0D3DC"));
	QAction* searchAction = new QAction(QIcon(searchIcon), "", searchBox);
	searchBox->addAction(searchAction, QLineEdit::LeadingPosition);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(searchBox);
	layout->addWidget(projectListWidget);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	connect(searchBox, &QLineEdit::textChanged, this, [this](const QString& text) {
		proxyModel->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
		});

	connect(projectListWidget, &QTableView::clicked, this, &cp::ProjectList::SelectProject);
	connect(projectListWidget, &QTableView::doubleClicked, this, &cp::ProjectList::OpenProject);
}

void cp::ProjectList::PopulateProjectList(const std::vector<cp::Project>& projects)
{
	if (model) delete model;
	if (proxyModel) delete proxyModel;

	model = new QStandardItemModel(projects.size(), 3, this);
	proxyModel = new cp::FilterProxyModel();

	model->setHorizontalHeaderLabels({ "Name", "Modified", "Version" });
	proxyModel->setSourceModel(model);
	projectListWidget->setModel(proxyModel);

	projectListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	projectListWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	projectListWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	projectListWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	projectListWidget->verticalHeader()->setVisible(false);
	projectListWidget->setShowGrid(false);
	projectListWidget->setAlternatingRowColors(true);
	projectListWidget->setStyleSheet(R"(
		QTableView {
			background-color: #1A1F2B;
			border-radius: 2px;
			outline: none;
			padding: 8px 0px;
			margin: 0px;
		}
		QTableView::item {
			border: none;
			outline: none;
			background-color: transparent;
			margin: 2px;
		}
		QHeaderView::section {
			background-color: #23283A;
			color: #D0D3DC;
			border: none;
			padding: 4px 8px;
		}
		QTableView::item:selected {
			background-color: #3E465A;
			color: #FFFFFF;
		}
	)");

	projectListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	projectListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	projectListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	projectListWidget->setColumnWidth(1, 140);
	projectListWidget->setColumnWidth(2, 105);

	for (size_t i = 0; i < projects.size(); i++)
	{
		AddProject(i, projects[i]);
	}
}

void cp::ProjectList::AddProject(const size_t _index, const cp::Project& _project)
{
	QStandardItem* item = new QStandardItem(QString::fromStdString(_project.name));
	item->setToolTip(QString::fromStdString(_project.path));
	model->setItem(_index, 0, item);
	QStandardItem* lastOpenedItem = new QStandardItem(QString::fromStdString(_project.FormatLastOpened()));
	model->setItem(_index, 1, lastOpenedItem);
	QStandardItem* versionItem = new QStandardItem(QString::fromStdString(_project.engineVersion.ToString()));
	model->setItem(_index, 2, versionItem);
}

void cp::ProjectList::AddProjectFocusedListener(std::function<void(const std::string&)> callback)
{
	connect(this, &ProjectList::ProjectFocused, [=](const std::string& _projectPath) { callback(_projectPath); });
}

void cp::ProjectList::AddProjectOpenedListener(std::function<void(const std::string&)> callback)
{
	connect(this, &ProjectList::ProjectOpened, [=](const std::string& _projectPath) { callback(_projectPath); });
}

void cp::ProjectList::SelectProject(const QModelIndex& index)
{
	QModelIndex sourceIndex = proxyModel->mapToSource(index);
	QStandardItem* item = model->item(sourceIndex.row(), 0);

	if (item)
	{
		QString projectPath = item->toolTip();
		emit ProjectFocused(projectPath.toStdString());
	}
}

void cp::ProjectList::OpenProject(const QModelIndex& index)
{
	QModelIndex sourceIndex = proxyModel->mapToSource(index);
	QStandardItem* item = model->item(sourceIndex.row(), 0);

	if (item)
	{
		QString projectPath = item->toolTip();
		emit ProjectOpened(projectPath.toStdString());
	}
}
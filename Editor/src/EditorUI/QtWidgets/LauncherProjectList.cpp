#include "pch.hpp"

#include "LauncherProjectList.hpp"

#include "EditorUI/QtWidgets/Helper.hpp"

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
#include <QProxyStyle>
#include <QMouseEvent>
#include <QItemSelectionModel>
#include <QVariant>

bool cp::FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	return sourceModel()->data(index).toString().contains(filterRegularExpression());
}

cp::ProjectListModel::ProjectListModel(QObject* parent)
	: QAbstractListModel(parent)
{
}

int cp::ProjectListModel::rowCount(const QModelIndex&) const
{
	return static_cast<int>(m_projects.size());
}

QVariant cp::ProjectListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() >= m_projects.size())
		return {};

	const auto& project = m_projects[index.row()];

	switch (role)
	{
	case Qt::DisplayRole:
		return QString::fromStdString(project.name);

	case Qt::ToolTipRole:
		return QString::fromStdString(project.path);

	case Qt::UserRole:
		return QVariant::fromValue(&project);
	}
	return {};
}

void cp::ProjectListModel::setProjects(const std::vector<Project>& projects)
{
	beginResetModel();
	m_projects = projects;
	endResetModel();
}

const cp::Project& cp::ProjectListModel::getProject(int row) const
{
	return m_projects[row];
}

void cp::ProjectListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	bool isSelected = option.state & QStyle::State_Selected;
	bool isHovered = option.state & QStyle::State_MouseOver;

	QVariant projectVariant = index.data(Qt::UserRole);
	const cp::Project* project = projectVariant.value<const cp::Project*>();

	painter->save();

	QRect r = option.rect;
	painter->fillRect(r, QColor("#1A1F2B"));

	QRect borderLeftRect = r.adjusted(0, 0, 3 - r.width(), 0);
	painter->fillRect(borderLeftRect, isSelected ? QColor("#A66BFF") : QColor("#333"));

	QRect versionRect = r.adjusted(r.width() - 95, r.height() / 2 - 7, -25, -r.height() / 2 + 7);
	painter->setPen(QColor("#D0D3DC"));
	QFont versionFont = option.font;
	versionFont.setPointSize(10);
	painter->setFont(versionFont);
	painter->drawText(versionRect, QString::fromStdString(project->engineVersion.ToString()));

	QRect modifiedRect = r.adjusted(r.width() - 220, r.height() / 2 - 7, -105, -r.height() / 2 + 7);
	painter->setPen(QColor("#D0D3DC"));
	QFont modifiedFont = option.font;
	modifiedFont.setPointSize(10);
	painter->setFont(modifiedFont);
	std::string modifiedText = Helper::Time::FormatTimeSince(project->lastOpened, static_cast<uint64_t>(std::time(nullptr)));
	painter->drawText(modifiedRect, QString::fromStdString(modifiedText));

	QString title = QString::fromStdString(project->name);
	painter->setPen(isSelected ? QColor("#A66BFF") : Qt::white);
	QFont titleFont = option.font;
	titleFont.setPointSize(16);
	titleFont.setBold(true);
	painter->setFont(titleFont);

	if(isSelected || isHovered)
		painter->drawText(r.adjusted(20, 8, -200, -r.height() + 30), title);
	else
		painter->drawText(r.adjusted(20, r.height() / 2 - 12, -200, -r.height() / 2 + 12), title);

	if(isSelected || isHovered)
	{
		QString path = QString::fromStdString(project->path);
		painter->setPen(QColor("#A66BFF").darker(180));
		QFont pathFont = option.font;
		pathFont.setPointSize(10);
		painter->setFont(pathFont);
		painter->drawText(r.adjusted(20, r.height() / 2 + 5, -100, -5), path);
	}

	painter->restore();
}

QSize cp::ProjectListItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QSize(option.rect.width(), 60);
}

cp::ProjectListTreeWidget::ProjectListTreeWidget(QWidget* parent) : QListView(parent)
{
	setItemDelegate(new ProjectListItemDelegate(this));
}

cp::ProjectList::ProjectList(QWidget* parent) : QWidget(parent)
{
	projectListView = new ProjectListTreeWidget(this);
	projectModel = new ProjectListModel(this);
	proxyModel = new cp::FilterProxyModel();
	proxyModel->setSourceModel(projectModel);
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

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

	projectListView->setModel(proxyModel);
	projectListView->setSelectionMode(QAbstractItemView::SingleSelection);
	projectListView->setViewMode(QListView::ListMode);
	projectListView->setUniformItemSizes(false);
	projectListView->setSpacing(8);
	projectListView->setStyleSheet(R"(
		QListView {
			background-color: #3E465A;
			outline: none;
			padding: 8px 0px;
		}

		QListView::item {
			background-color: transparent;
			border-left: 2px solid #333;
			margin: 2px;
		}

		QListView::item:selected {
			background-color: transparent;
			border-left: 2px solid #5A96FF;
			color: white;
		}
	)");

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(searchBox);
	layout->addWidget(projectListView);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	connect(searchBox, &QLineEdit::textChanged, this, [this](const QString& text) {
		proxyModel->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
	});

	connect(projectListView, &QListView::clicked, this, &cp::ProjectList::SelectProject);
	connect(projectListView, &QListView::doubleClicked, this, &cp::ProjectList::OpenProject);
}

void cp::ProjectList::PopulateProjectList(const std::vector<cp::Project>& projects)
{
	projectModel->setProjects(projects);
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
	QModelIndex source = proxyModel->mapToSource(index);
	const auto& project = projectModel->getProject(source.row());

	emit ProjectFocused(project.path);
}

void cp::ProjectList::OpenProject(const QModelIndex& index)
{
	QModelIndex source = proxyModel->mapToSource(index);
	const auto& project = projectModel->getProject(source.row());

	emit ProjectOpened(project.path);
}
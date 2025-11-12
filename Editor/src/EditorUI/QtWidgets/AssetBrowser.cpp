#include "AssetBrowser.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileIconProvider>
#include <QLabel>
#include <QDebug>

#include "Inspector.hpp"

cp::AssetBrowserWidget::AssetBrowserWidget(const QString& rootPath, QWidget* parent)
    : QWidget(parent)
    , rootPath(rootPath)
{
    SetupUI();
    SetupConnections();
    SetRootPath(rootPath);
}

void cp::AssetBrowserWidget::SetupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));

    viewModeCombo = new QComboBox(this);
    viewModeCombo->addItem("Tree", static_cast<int>(ViewMode::Tree));
    viewModeCombo->addItem("List", static_cast<int>(ViewMode::List));
    viewModeCombo->addItem("Small Icons", static_cast<int>(ViewMode::SmallIcons));
    viewModeCombo->addItem("Large Icons", static_cast<int>(ViewMode::LargeIcons));

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");

    toolbar->addWidget(new QLabel("View: "));
    toolbar->addWidget(viewModeCombo);
    toolbar->addSeparator();
    toolbar->addWidget(searchBar);

    layout->addWidget(toolbar);

    // --- File model ---
    model = new QFileSystemModel(this);
    model->setRootPath(rootPath);
    model->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    // --- Views ---
    treeView = new QTreeView(this);
    treeView->setModel(model);
    treeView->setRootIndex(model->index(rootPath));
    treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    listView = new QListView(this);
    listView->setModel(model);
    listView->setRootIndex(model->index(rootPath));
    listView->setViewMode(QListView::ListMode);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);

    iconView = new QListView(this);
    iconView->setModel(model);
    iconView->setRootIndex(model->index(rootPath));
    iconView->setViewMode(QListView::IconMode);
    iconView->setIconSize(QSize(32, 32));
    iconView->setGridSize(QSize(80, 80));
    iconView->setResizeMode(QListView::Adjust);

    largeIconView = new QListView(this);
    largeIconView->setModel(model);
    largeIconView->setRootIndex(model->index(rootPath));
    largeIconView->setViewMode(QListView::IconMode);
    largeIconView->setIconSize(QSize(96, 96));
    largeIconView->setGridSize(QSize(128, 128));
    largeIconView->setResizeMode(QListView::Adjust);

    // --- Stack (to switch between layouts easily) ---
    stack = new QStackedWidget(this);
    stack->addWidget(treeView);
    stack->addWidget(listView);
    stack->addWidget(iconView);
    stack->addWidget(largeIconView);

    layout->addWidget(stack);
    setLayout(layout);
}

void cp::AssetBrowserWidget::SetupConnections()
{
    connect(viewModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &cp::AssetBrowserWidget::OnViewModeChanged);

    connect(searchBar, &QLineEdit::textChanged,
        this, &cp::AssetBrowserWidget::OnSearchTextChanged);

    connect(treeView, &QTreeView::clicked,
        this, &cp::AssetBrowserWidget::OnItemActivated);
    connect(listView, &QListView::clicked,
        this, &cp::AssetBrowserWidget::OnItemActivated);
    connect(iconView, &QListView::clicked,
        this, &cp::AssetBrowserWidget::OnItemActivated);
    connect(largeIconView, &QListView::clicked,
        this, &cp::AssetBrowserWidget::OnItemActivated);
}

void cp::AssetBrowserWidget::SetRootPath(const QString& path)
{
    rootPath = path;
    QModelIndex rootIndex = model->setRootPath(rootPath);

	std::vector<QAbstractItemView*> views = { treeView, listView, iconView, largeIconView };

    for (auto view : views)
        view->setRootIndex(rootIndex);
}

void cp::AssetBrowserWidget::SetViewMode(ViewMode mode)
{
    currentMode = mode;
    ApplyViewMode(mode);
    viewModeCombo->setCurrentIndex(static_cast<int>(mode));
}

void cp::AssetBrowserWidget::OnViewModeChanged(int index)
{
    ViewMode mode = static_cast<ViewMode>(viewModeCombo->itemData(index).toInt());
    SetViewMode(mode);
}

void cp::AssetBrowserWidget::ApplyViewMode(ViewMode mode)
{
    switch (mode)
    {
    case ViewMode::Tree: stack->setCurrentWidget(treeView); break;
    case ViewMode::List: stack->setCurrentWidget(listView); break;
    case ViewMode::SmallIcons: stack->setCurrentWidget(iconView); break;
    case ViewMode::LargeIcons: stack->setCurrentWidget(largeIconView); break;
    }
}

void cp::AssetBrowserWidget::OnSearchTextChanged(const QString& text)
{
    model->setNameFilters(text.isEmpty() ? QStringList() : QStringList{ "*" + text + "*" });
    model->setNameFilterDisables(false);
}

void cp::AssetBrowserWidget::OnItemActivated(const QModelIndex& index)
{
    QString filePath = model->filePath(index);
    emit FileActivated(filePath);
	LOG_DEBUG(MF("File activated: ", filePath.toStdString()));
}

void cp::AssetBrowserWidget::LinkToInspector(cp::Inspector* inspector)
{
    connect(this, &cp::AssetBrowserWidget::FileActivated, [=](const QString& path)
        {
            QFileInfo fileInfo(path);
			LOG_DEBUG(MF("Linking to inspector for file: ", path.toStdString()));

            if (fileInfo.isFile())
            {
				LOG_DEBUG(MF("Showing file in inspector: ", path.toStdString()));
                inspector->ShowFile(path.toStdString());
            }
        });
}
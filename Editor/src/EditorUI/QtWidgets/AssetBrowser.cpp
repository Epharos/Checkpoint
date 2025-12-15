#include "AssetBrowser.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileIconProvider>
#include <QLabel>
#include <QDebug>

#include "Inspector.hpp"
#include "SceneHierarchy.hpp"
#include "CheckpointEditor.hpp"

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
	layout->setSizeConstraint(QLayout::SetMinimumSize);

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
	model->setReadOnly(false);

    // --- Views ---
    treeView = new QTreeView(this);
    treeView->setModel(model);
    treeView->setRootIndex(model->index(rootPath));
    treeView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
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

    // TMP CONTEXT MENU

    QMenu* contextMenuFolder = new QMenu(treeView);
    QAction* createMaterial = new QAction("Create Material", treeView);
    contextMenuFolder->addAction(createMaterial);

    QMenu* contextMenuMaterial = new QMenu(treeView);
    QAction* createInstance = new QAction("Create Instance", treeView);
    contextMenuMaterial->addAction(createInstance);

    QString* rcPath = new QString();

    connect(createMaterial, &QAction::triggered, [=] {
        cp::Material mat(&cp::CheckpointEditor::VulkanCtx);
        cp::JsonSerializer serializer;
        mat.Serialize(serializer);

        QString matFileName = QString::fromStdString(rcPath->toStdString() + "\\New_Material.mat");
        QFileInfo fileInfo(matFileName);

        uint16_t tryIndex = 0;

        while (fileInfo.exists())
        {
            matFileName = QString::fromStdString(rcPath->toStdString() + "\\New_Material_" + std::to_string(tryIndex) + ".mat");
            tryIndex++;
            fileInfo = QFileInfo(matFileName);
        }

        serializer.Write(matFileName.toStdString());

        QModelIndex index = model->index(matFileName);

        if (index.isValid())
        {
            treeView->edit(index);
        }
        });

    connect(createInstance, &QAction::triggered, [=] {
        cp::MaterialInstance matInstance(&cp::CheckpointEditor::VulkanCtx);
        cp::JsonSerializer serializer;
        matInstance.SetAssociatedMaterial(rcPath->toStdString());
        matInstance.Serialize(serializer);

        QFileInfo matFileInfo(*rcPath);
        QString instanceFileName = QString::fromStdString(matFileInfo.path().append("\\").append(matFileInfo.baseName()).toStdString() + ".matinstance");
        QFileInfo fileInfo(instanceFileName);

        uint16_t tryIndex = 0;

        while (fileInfo.exists())
        {
            instanceFileName = QString::fromStdString(matFileInfo.path().append("\\").append(matFileInfo.baseName().toStdString() + "_" + std::to_string(tryIndex)).toStdString() + ".matinstance");
            tryIndex++;
            fileInfo = QFileInfo(instanceFileName);
        }

        std::ofstream file(instanceFileName.toStdString(), std::ios::binary);
        file.close(); //We just create the file

        serializer.Write(instanceFileName.toStdString());

        QModelIndex index = model->index(instanceFileName);

        if (index.isValid())
        {
            treeView->edit(index);
        }
        });

    connect(treeView, &QTreeView::customContextMenuRequested, [=](QPoint _point)
        {
            QModelIndex index = treeView->indexAt(_point);

            *rcPath = model->filePath(index);
            QFileInfo fileInfo(*rcPath);

            if (fileInfo.isDir())
            {
                contextMenuFolder->popup(treeView->viewport()->mapToGlobal(_point));
            }

            if (fileInfo.isFile())
            {
                if (fileInfo.suffix().endsWith("mat"))
                {
                    contextMenuMaterial->popup(treeView->viewport()->mapToGlobal(_point));
                }
            }
        });

    // END TMP
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

    connect(treeView, &QTreeView::doubleClicked,
		this, &cp::AssetBrowserWidget::OnItemSelected);
	connect(listView, &QListView::doubleClicked,
		this, &cp::AssetBrowserWidget::OnItemSelected);
	connect(iconView, &QListView::doubleClicked,
		this, &cp::AssetBrowserWidget::OnItemSelected);
	connect(largeIconView, &QListView::doubleClicked,
		this, &cp::AssetBrowserWidget::OnItemSelected);
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
}

void cp::AssetBrowserWidget::OnItemSelected(const QModelIndex& index)
{
    QString filePath = model->filePath(index);
	emit FileSelected(filePath);
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

void cp::AssetBrowserWidget::LinkToSceneHierarchy(cp::SceneHierarchy* sceneHierarchy)
{
    connect(this, &cp::AssetBrowserWidget::FileSelected, [=](const QString& path)
        {
            if (path.endsWith(".cpscene"))
            {
                cp::SceneAsset* newScene = new cp::SceneAsset();
                newScene->path = path.toStdString();

                cp::JsonSerializer serializer;
                serializer.Read(path.toStdString());
                newScene->Deserialize(serializer);

                cp::SceneAsset* oldScene = cp::CheckpointEditor::CurrentScene;
                cp::CheckpointEditor::CurrentScene = newScene;
                delete oldScene;

				sceneHierarchy->InitTree(newScene);
            }
        });
}

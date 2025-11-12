#pragma once

#include <QFileSystemModel>
#include <QListView>
#include <QTreeView>
#include <QToolBar>
#include <QStackedWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QDir>

namespace cp {
	class Inspector;

    class AssetBrowserWidget : public QWidget
    {
#ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
#endif

    public:
        enum class ViewMode
        {
            Tree,
            List,
            SmallIcons,
            LargeIcons
        };

        explicit AssetBrowserWidget(const QString& rootPath, QWidget* parent = nullptr);

        void SetRootPath(const QString& path);
        void SetViewMode(ViewMode mode);
		void LinkToInspector(cp::Inspector* inspector);

    signals:
        void FileActivated(const QString& filePath);
        void FileSelected(const QString& filePath);

    private slots:
        void OnViewModeChanged(int index);
        void OnSearchTextChanged(const QString& text);
        void OnItemActivated(const QModelIndex& index);

    private:
        void SetupUI();
        void SetupConnections();
        void ApplyViewMode(ViewMode mode);

    private:
        QFileSystemModel* model = nullptr;
        QTreeView* treeView = nullptr;
        QListView* listView = nullptr;
        QListView* iconView = nullptr;
        QListView* largeIconView = nullptr;

        QToolBar* toolbar = nullptr;
        QComboBox* viewModeCombo = nullptr;
        QLineEdit* searchBar = nullptr;

        QStackedWidget* stack = nullptr;

        QString rootPath;
        ViewMode currentMode = ViewMode::List;
    };
}
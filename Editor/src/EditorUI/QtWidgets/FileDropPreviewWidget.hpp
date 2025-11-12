#pragma once

#include "pch.hpp"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QPainter>
#include <QPixmap>
#include <QFileInfo>
#include <QDirIterator>
#include <QMouseEvent>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>

#include <string>

namespace cp {
    class FileDropPreviewWidget : public QWidget {
    #ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
    #endif
    public:
        FileDropPreviewWidget(std::string* value, const QString& label, const QStringList& extensions, const QString& rootDir, QWidget* parent = nullptr)
            : QWidget(parent), valuePtr(value), labelText(label), extensions(extensions), rootDir(rootDir)
        {
            setAcceptDrops(true);
            setFixedHeight(80);

            auto* mainLayout = new QHBoxLayout(this);
            mainLayout->setContentsMargins(8, 8, 8, 8);
            mainLayout->setSpacing(8);

            auto* rightWidget = new QWidget(this);
            auto* rightLayout = new QVBoxLayout(rightWidget);
            rightLayout->setContentsMargins(0, 6, 0, 6);
            rightLayout->setSpacing(2);

            labelWidget = new QLabel(label, rightWidget);
            labelWidget->setStyleSheet("font-weight: bold; font-size: 12px;");
            rightLayout->addWidget(labelWidget);

            infoWidget = new QLabel(rightWidget);
            infoWidget->setStyleSheet("font-size: 12px; color: #A66BFF;");
            rightLayout->addWidget(infoWidget);
            rightLayout->addStretch(1);
        
            iconWidget = new QLabel(this);
            iconWidget->setFixedSize(64, 64);
            iconWidget->setAlignment(Qt::AlignCenter);
            iconWidget->setStyleSheet("background: #2F3545; border: 1px solid #3E465A; border-radius: 6px;");
            mainLayout->addWidget(iconWidget, 0);
            mainLayout->addWidget(rightWidget, 1);

            setLayout(mainLayout);

            UpdateDisplay();
        }

        virtual void SetFile(const QString& filePath) {
            if (valuePtr) *valuePtr = filePath.toStdString();
		    currentFilePath = filePath.toStdString();
            UpdateDisplay();
            emit FileSelected(filePath);
        }

    protected:
        void dragEnterEvent(QDragEnterEvent* event) override {
            if (event->mimeData()->hasUrls()) {
                for (const QUrl& url : event->mimeData()->urls()) {
                    if (IsFileAcceptable(url.toLocalFile())) {
                        event->acceptProposedAction();
                        return;
                    }
                }
            }
            event->ignore();
        }

        void dropEvent(QDropEvent* event) override {
            for (const QUrl& url : event->mimeData()->urls()) {
                QString path = url.toLocalFile();
                if (IsFileAcceptable(path)) {
                    SetFile(path);
                    break;
                }
            }
        }

        void mousePressEvent(QMouseEvent* event) override {
            if (event->button() == Qt::LeftButton) {
                // Ouvre la popup de sélection
                QString selected = OpenFilePopup();
                if (!selected.isEmpty()) {
                    SetFile(selected);
                }
            }
        }

        bool IsFileAcceptable(const QString& path) const {
            QFileInfo info(path);
            if (!info.isFile()) return false;
            if (extensions.isEmpty()) return true;
            for (const QString& ext : extensions) {
                if (info.suffix().compare(ext, Qt::CaseInsensitive) == 0)
                    return true;
            }
            return false;
        }

        QString OpenFilePopup() {
            LOG_DEBUG(MF("Opening file selection popup for directory : ", rootDir.toStdString()));

            QStringList files = FindFilesRecursively(rootDir, extensions);
            if (files.isEmpty()) return "";

            QDialog dialog(this);
            dialog.setWindowTitle("Select File");
            QVBoxLayout vlayout(&dialog);
            QListWidget list(&dialog);
            for (const QString& file : files)
                list.addItem(file);
            vlayout.addWidget(&list);
            QPushButton okBtn("OK", &dialog);
            vlayout.addWidget(&okBtn);
            QObject::connect(&okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
            QObject::connect(&list, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
            if (dialog.exec() == QDialog::Accepted && list.currentItem()) {
                return list.currentItem()->text();
            }
            return "";
        }

        QStringList FindFilesRecursively(const QString& dir, QStringList filters) {
            QStringList result;
        
		    for (auto& ext : filters) {
                if (!ext.startsWith("*"))
                    ext = "*" + ext;
            }

            QDirIterator it(dir, filters, QDir::Files, QDirIterator::Subdirectories);

            while (it.hasNext())
            {
			    std::string fullPath = it.next().toStdString();
			    //fullPath = Project::GetResourceRelativePath(fullPath);
			    result << QString::fromStdString(fullPath);
            }

            return result;
        }

        void UpdateDisplay() {
            if (!currentFilePath.empty()) {
                QFileInfo info(QString::fromStdString(currentFilePath));
                infoWidget->setText(info.fileName());
                if (info.suffix().compare("png", Qt::CaseInsensitive) == 0 ||
                    info.suffix().compare("jpg", Qt::CaseInsensitive) == 0 ||
                    info.suffix().compare("jpeg", Qt::CaseInsensitive) == 0) {
                    QPixmap pix(QString::fromStdString(currentFilePath));
                    iconWidget->setPixmap(pix.scaled(iconWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
                else {
                    iconWidget->setPixmap(QPixmap(":/icons/file.svg").scaled(iconWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
            else {
                infoWidget->setText(extensions.join(", "));
                iconWidget->setPixmap(QPixmap(":/icons/file.svg").scaled(iconWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }

    signals:
        void FileSelected(const QString& path);

    private:
        std::string* valuePtr;
	    std::string currentFilePath;
        QString labelText;
        QStringList extensions;
        QString rootDir;
        QLabel* labelWidget = nullptr;
        QLabel* infoWidget = nullptr;
        QLabel* iconWidget = nullptr;
    };

    template<class T>
    class ResourceDropPreviewWidget : public FileDropPreviewWidget {
        public:
            ResourceDropPreviewWidget(std::shared_ptr<T>* value, const QString& label, const QStringList& extensions, const QString& rootDir, QWidget* parent = nullptr)
                : FileDropPreviewWidget(nullptr, label, extensions, rootDir, parent), resourcePtr(value)
            {
                connect(this, &FileDropPreviewWidget::FileSelected, this, &ResourceDropPreviewWidget::OnFileSelected);
                UpdateDisplay();
	        }

        private:
            void OnFileSelected(const QString& path) {
                if (resourcePtr) {
                    *resourcePtr = cp::ResourceManager::Get()->GetOrLoad<T>(path.toStdString());
                }
	        }

        protected:
	        std::shared_ptr<T>* resourcePtr;
    };

    class MeshDropPreviewWidget : public ResourceDropPreviewWidget<cp::Mesh> {
    #ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
    #endif

        public:
            MeshDropPreviewWidget(std::shared_ptr<cp::Mesh>* value, const QString& label, const QString& rootDir, QWidget* parent = nullptr)
                : ResourceDropPreviewWidget<cp::Mesh>(value, label, { "fbx", "obj", "gltf" }, rootDir, parent)
            {}
    };

    class TextureDropPreviewWidget : public ResourceDropPreviewWidget<cp::Texture> {
    #ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
    #endif

        public:
            TextureDropPreviewWidget(std::shared_ptr<cp::Texture>* value, const QString& label, const QString& rootDir, QWidget* parent = nullptr)
                : ResourceDropPreviewWidget<cp::Texture>(value, label, { "png", "jpg", "jpeg", "bmp" }, rootDir, parent)
            {}
    };

    class MaterialInstanceDropPreviewWidget : public ResourceDropPreviewWidget<cp::MaterialInstance> {
    #ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
    #endif

        public:
            MaterialInstanceDropPreviewWidget(std::shared_ptr<cp::MaterialInstance>* value, const QString& label, const QString& rootDir, QWidget* parent = nullptr)
                : ResourceDropPreviewWidget<cp::MaterialInstance>(value, label, { "matinstance" }, rootDir, parent)
            {}
    };
}
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

        updateDisplay();
    }

    void setFile(const QString& filePath) {
        if (valuePtr) *valuePtr = filePath.toStdString();
        updateDisplay();
        emit FileSelected(filePath);
    }

protected:
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasUrls()) {
            for (const QUrl& url : event->mimeData()->urls()) {
                if (isFileAcceptable(url.toLocalFile())) {
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
            if (isFileAcceptable(path)) {
                setFile(path);
                break;
            }
        }
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            // Ouvre la popup de sélection
            QString selected = openFilePopup();
            if (!selected.isEmpty()) {
                setFile(selected);
            }
        }
    }

    bool isFileAcceptable(const QString& path) const {
        QFileInfo info(path);
        if (!info.isFile()) return false;
        if (extensions.isEmpty()) return true;
        for (const QString& ext : extensions) {
            if (info.suffix().compare(ext, Qt::CaseInsensitive) == 0)
                return true;
        }
        return false;
    }

    QString openFilePopup() {
        QStringList files = findFilesRecursively(rootDir, extensions);
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

    QStringList findFilesRecursively(const QString& dir, QStringList filters) {
        QStringList result;
        
		for (auto& ext : filters) {
            if (!ext.startsWith("*"))
                ext = "*" + ext;
        }

        QDirIterator it(dir, filters, QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
			std::string fullPath = it.next().toStdString();
			fullPath = Project::GetResourceRelativePath(fullPath);
			result << QString::fromStdString(fullPath);
        }

        return result;
    }

    void updateDisplay() {
        if (valuePtr && !valuePtr->empty()) {
            QFileInfo info(QString::fromStdString(*valuePtr));
            infoWidget->setText(info.fileName());
            if (info.suffix().compare("png", Qt::CaseInsensitive) == 0 ||
                info.suffix().compare("jpg", Qt::CaseInsensitive) == 0 ||
                info.suffix().compare("jpeg", Qt::CaseInsensitive) == 0) {
                QPixmap pix(QString::fromStdString(*valuePtr));
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
    QString labelText;
    QStringList extensions;
    QString rootDir;
    QLabel* labelWidget = nullptr;
    QLabel* infoWidget = nullptr;
    QLabel* iconWidget = nullptr;
};
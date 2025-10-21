#pragma once
#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qdockwidget.h>
#include <QtGui/qevent.h>
#include <QtCore/qmimedata.h>

class DropMainWindow : public QMainWindow {
#ifndef BUILDING_PLUGIN_LOADER
    Q_OBJECT
#endif
public:
    DropMainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setAcceptDrops(true);
    }

protected:
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasFormat("application/x-dockwidget-ptr")) {
            event->acceptProposedAction();
        }
        else {
            QMainWindow::dragEnterEvent(event);
        }
    }

    void dropEvent(QDropEvent* event) override {
        auto md = event->mimeData();

        if (!md->hasFormat("application/x-dockwidget-ptr")) {
            QMainWindow::dropEvent(event);
            return;
        }

        QByteArray ba = md->data("application/x-dockwidget-ptr");
        if (ba.size() != sizeof(quintptr)) return;

        quintptr ptr = 0;
        memcpy(&ptr, ba.data(), sizeof(quintptr));

        QDockWidget* dock = reinterpret_cast<QDockWidget*>(ptr);
        if (!dock) return;

        QWidget* oldParent = dock->parentWidget();
        if (oldParent && oldParent != this) {
            dock->setParent(nullptr);
        }

        addDockWidget(Qt::LeftDockWidgetArea, dock);
        dock->show();

        event->acceptProposedAction();
    }
};
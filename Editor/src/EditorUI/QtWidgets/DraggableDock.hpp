#pragma once

#include <QtWidgets/qdockwidget.h>
#include <QtCore/qmimedata.h>
#include <QtGui/qevent.h>
#include <QtGui/qdrag.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
namespace cp {
    class DragTitleBar : public QWidget {
#ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
#endif
    public:
        explicit DragTitleBar(QDockWidget* parentDock) : QWidget(parentDock), dock(parentDock) {
            setAutoFillBackground(true);
            auto layout = new QHBoxLayout(this);
            label = new QLabel(parentDock->windowTitle(), this);
            layout->addWidget(label);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(8);
            setLayout(layout);
            label->setStyleSheet("QLabel { color: #D0D3DC; font-weight: bold; padding: 4px 8px; }");
        }
        void SetLabel(const QString& t) { label->setText(t); }

    protected:
        void mousePressEvent(QMouseEvent* ev) override {
            lastPos = ev->pos();
            //QWidget::mousePressEvent(ev);
        }
        void mouseMoveEvent(QMouseEvent* ev) override {
            if ((ev->buttons() & Qt::LeftButton) && (ev->pos() - lastPos).manhattanLength() > 6) {
                QDrag* drag = new QDrag(this);
                QMimeData* mime = new QMimeData();

                quintptr ptr = reinterpret_cast<quintptr>(dock);
                QByteArray ba;
                ba.resize(sizeof(quintptr));
                memcpy(ba.data(), &ptr, sizeof(quintptr));

                mime->setData("application/x-dockwidget-ptr", ba);
                drag->setMimeData(mime);
                drag->exec(Qt::MoveAction);
            }

            //QWidget::mouseMoveEvent(ev);
        }
    private:
        QLabel* label;
        QPoint lastPos;
        QDockWidget* dock;
    };

    class DraggableDockWidget : public QDockWidget {
#ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
#endif
    public:
        DraggableDockWidget(const QString& title, QWidget* parent = nullptr)
            : QDockWidget(title, parent)
        {
            titleBar_ = new DragTitleBar(this);
            setTitleBarWidget(titleBar_);
            setAllowedAreas(Qt::AllDockWidgetAreas);
        }

    protected:
        DragTitleBar* titleBar_ = nullptr;
    };
};
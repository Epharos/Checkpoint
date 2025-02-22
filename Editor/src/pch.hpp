#pragma once

#include <PreProcessor.hpp>
#include <Core.hpp>
#include <ECS.hpp>

#include <iostream>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qwidget.h>

#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qdockwidget.h>

#include <QtWidgets/qboxlayout.h>

#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qtreeview.h>

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qfilesystemmodel.h>

#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

#include <QtCore/qfile.h>
#include <QtCore/qtimer.h>

#include <QtGui/qvulkaninstance.h>

struct ProjectData
{
    QString name;
    QString path;
    QDateTime lastOpened;

    bool operator==(const ProjectData& other) const
    {
        return name == other.name && path == other.path;
    }

    bool operator!=(const ProjectData& other) const
    {
        return !(*this == other);
    }

    bool operator<(const ProjectData& other) const
    {
        return lastOpened > other.lastOpened;
    }

    ProjectData(const QString& name, const QString& path, const QDateTime& lastOpened) : name(name), path(path), lastOpened(lastOpened) {}
    ProjectData() : name(""), path(""), lastOpened(QDateTime::currentDateTime()) {}
    ProjectData(const ProjectData& _other) : name(_other.name), path(_other.path), lastOpened(_other.lastOpened) {}
};
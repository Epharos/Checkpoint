#pragma once

#include <PreProcessor.hpp>

#include <iostream>
#include <memory>

#include <Core.hpp>
#include <ECS.hpp>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qwidgetaction.h>

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
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qfilesystemmodel.h>

#include <QtWidgets/qgraphicseffect.h>
#include <QtCore/qpropertyanimation.h>

#include <QtWidgets/qtableview.h>
#include <QtGui/qstandarditemmodel.h>

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
    QDateTime creationDate;
    QDateTime lastOpened;
    uint32_t engineVersion;

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

    ProjectData(const QString& name, const QString& path, const QDateTime& lastOpened, const uint32_t _version) : name(name), path(path), lastOpened(lastOpened), engineVersion(_version) {}
    ProjectData() : name(""), path(""), lastOpened(QDateTime::currentDateTime()), engineVersion(ENGINE_VERSION) {}
    ProjectData(const ProjectData& _other) : name(_other.name), path(_other.path), lastOpened(_other.lastOpened), engineVersion(_other.engineVersion) {}

	static ProjectData FromJson(const QJsonObject& obj)
	{
		ProjectData data;
		data.name = obj["name"].toString();
		data.path = obj["path"].toString();
		data.lastOpened = QDateTime::fromString(obj["lastOpened"].toString(), "yyyy-MM-dd HH:mm");
		data.creationDate = QDateTime::fromString(obj["creationDate"].toString(), "yyyy-MM-dd HH:mm");
		data.engineVersion = obj["engineVersion"].toInt();
		return data;
	}

	static QJsonObject ToJson(const ProjectData& data)
	{
		QJsonObject obj;
		obj["name"] = data.name;
		obj["path"] = data.path;
		obj["lastOpened"] = data.lastOpened.toString("yyyy-MM-dd HH:mm");
		obj["creationDate"] = data.creationDate.toString("yyyy-MM-dd HH:mm");
		obj["engineVersion"] = static_cast<qint64>(data.engineVersion);
		return obj;
	}
};
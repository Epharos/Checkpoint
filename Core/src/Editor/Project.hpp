#pragma once

#include "pch.hpp"
#include "../Context/VulkanContext.hpp"
#include <QtCore/qdatetime.h>
#include <QtCore/qjsonobject.h> //TODO : Switch to newtonsoft json

namespace cp
{
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

	struct Project
	{
		static ProjectData data;

		static std::string GetProjectPath()
		{
			return Project::data.path.toStdString();
		}

		static std::string GetResourcePath()
		{
			return Project::GetProjectPath() + "/Resources";
		}

		static std::string GetResourceRelativePath(const std::string& _resource)
		{
			return _resource.substr(Project::GetResourcePath().size());
		}
	};
}
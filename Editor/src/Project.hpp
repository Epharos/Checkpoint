#pragma once

#include "pch.hpp"

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

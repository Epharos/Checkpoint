#pragma once

#include <string>
#include <vector>

namespace cp
{
	struct LocalLibraryDependency
	{
		std::string includePath;
		std::string libPath;
	};

	struct GitDependency
	{
		std::string repo;
		std::string tag;
		std::string linkTarget;
	};

	struct UserPluginDescriptor
	{
		std::string name;
		std::vector<std::string> pluginDeps;
		std::vector<std::string> engineLibs;
		std::vector<LocalLibraryDependency> localLibs;
		std::vector<GitDependency> githubDeps;
	};
}

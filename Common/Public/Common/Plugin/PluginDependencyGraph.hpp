#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace cp
{
	class PluginDependencyGraph
	{
	public:
		void AddPlugin(std::string name, std::vector<std::string> deps = {});
		void RemovePlugin(std::string_view name);
		void Clear();

		[[nodiscard]] std::vector<std::string> GetLoadOrder(std::string* outError = nullptr) const;
		[[nodiscard]] std::vector<std::string> GetAffectedSet(std::string_view dirtyPlugin) const;
		[[nodiscard]] std::vector<std::string> GetUnloadOrder(std::string_view dirtyPlugin) const;

		[[nodiscard]] bool HasPlugin(std::string_view name) const;

	private:
		struct Node
		{
			std::string name;
			std::vector<std::string> deps;
		};

		std::vector<Node> nodes;
	};
}

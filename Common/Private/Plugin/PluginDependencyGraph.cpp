#include "../../Public/Common/Plugin/PluginDependencyGraph.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace cp
{
	void PluginDependencyGraph::AddPlugin(std::string name, std::vector<std::string> deps)
	{
		RemovePlugin(name);
		nodes.push_back({ std::move(name), std::move(deps) });
	}

	void PluginDependencyGraph::RemovePlugin(std::string_view name)
	{
		std::erase_if(nodes, [&](const Node& n) { return n.name == name; });
	}

	void PluginDependencyGraph::Clear()
	{
		nodes.clear();
	}

	bool PluginDependencyGraph::HasPlugin(std::string_view name) const
	{
		return std::ranges::any_of(nodes, [&](const Node& n) { return n.name == name; });
	}

	std::vector<std::string> PluginDependencyGraph::GetLoadOrder(std::string* outError) const
	{
		std::unordered_map<std::string, int> inDegree;
		std::unordered_map<std::string, std::vector<std::string>> revEdges;

		for (const auto& node : nodes)
		{
			if (!inDegree.contains(node.name))
			{
				inDegree[node.name] = 0;
			}

			for (const auto& dep : node.deps)
			{
				revEdges[dep].push_back(node.name);
				++inDegree[node.name];

				if (!inDegree.contains(dep))
				{
					inDegree[dep] = 0;
				}
			}
		}

		std::queue<std::string> queue;
		for (const auto& [name, degree] : inDegree)
		{
			if (degree == 0)
			{
				queue.push(name);
			}
		}

		std::vector<std::string> result;
		result.reserve(inDegree.size());
		while (!queue.empty())
		{
			std::string current = queue.front();
			queue.pop();
			result.push_back(current);
			for (const auto& dependent : revEdges[current])
			{
				if (--inDegree[dependent] == 0)
				{
					queue.push(dependent);
				}
			}
		}

		if (result.size() != inDegree.size())
		{
			if (outError)
			{
				*outError = "Circular dependency detected in plugin graph";
			}

			return {};
		}

		return result;
	}

	std::vector<std::string> PluginDependencyGraph::GetAffectedSet(std::string_view dirtyPlugin) const
	{
		std::unordered_map<std::string, std::vector<std::string>> revEdges;
		for (const auto& node : nodes)
		{
			for (const auto& dep : node.deps)
			{
				revEdges[dep].push_back(node.name);
			}
		}

		std::unordered_set<std::string> visited;
		std::queue<std::string> queue;
		queue.emplace(dirtyPlugin);
		visited.emplace(dirtyPlugin);

		while (!queue.empty())
		{
			const std::string current = queue.front();
			queue.pop();
			for (const auto& dependent : revEdges[current])
			{
				if (visited.insert(dependent).second)
				{
					queue.push(dependent);
				}
			}
		}

		return { visited.begin(), visited.end() };
	}

	std::vector<std::string> PluginDependencyGraph::GetUnloadOrder(std::string_view dirtyPlugin) const
	{
		const std::vector<std::string> loadOrder = GetLoadOrder();
		const std::vector<std::string> affected = GetAffectedSet(dirtyPlugin);
		const std::unordered_set<std::string> affectedSet(affected.begin(), affected.end());

		std::vector<std::string> result;
		result.reserve(affected.size());
		for (const auto& name : loadOrder)
		{
			if (affectedSet.contains(name))
			{
				result.push_back(name);
			}
		}

		std::ranges::reverse(result);
		return result;
	}
}

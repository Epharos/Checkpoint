#include "PassDependencyManager.hpp"

#include <Common/Core/Assert.hpp>

#include <algorithm>
#include <cstdint>
#include <queue>
#include <unordered_set>

namespace cp
{
    namespace
    {
        using PassOrderMap = std::unordered_map<IRenderPass*, size_t>;
        using PassInDegreeMap = std::unordered_map<IRenderPass*, int32_t>;
        using PassAdjacencyMap = std::unordered_map<IRenderPass*, std::vector<IRenderPass*>>;

        /**
         * @brief Builds a stable registration index for each pass.
         * @param _passes Passes in frame-graph registration order.
         * @return A map from pass pointer to its registration index.
         */
        PassOrderMap BuildPassOrderMap(const std::vector<IRenderPass*>& _passes)
        {
            PassOrderMap passOrder;
            passOrder.reserve(_passes.size());

            for (size_t i = 0; i < _passes.size(); ++i)
            {
                passOrder[_passes[i]] = i;
            }

            return passOrder;
        }

        /**
         * @brief Initializes graph containers with one empty node per pass.
         * @param _passes All passes to include in the scheduling graph.
         * @param _indegree Output map initialized to zero indegree for each pass.
         * @param _adjacency Output map initialized with empty dependent lists.
         */
        void InitializeGraphNodes(
            const std::vector<IRenderPass*>& _passes,
            PassInDegreeMap& _indegree,
            PassAdjacencyMap& _adjacency
        )
        {
            _indegree.reserve(_passes.size());
            _adjacency.reserve(_passes.size());

            for (IRenderPass* pass : _passes)
            {
                _indegree[pass] = 0;
                _adjacency[pass] = {};
            }
        }

        /**
         * @brief Converts declared dependencies into graph edges and indegree counts.
         * @param _dependencies Declared dependency list.
         * @param _passOrder Set of passes participating in the current scheduling.
         * @param _indegree Indegree map updated for each dependent pass.
         * @param _adjacency Adjacency list updated with dependsOn -> dependent edges.
         */
        void AddGraphEdgesFromDependencies(
            const std::vector<PassDependency>& _dependencies,
            const PassOrderMap& _passOrder,
            PassInDegreeMap& _indegree,
            PassAdjacencyMap& _adjacency
        )
        {
            for (const auto& dep : _dependencies)
            {
                if (!_passOrder.contains(dep.dependentPass) || !_passOrder.contains(dep.dependsOn))
                {
                    continue;
                }

                _adjacency[dep.dependsOn].push_back(dep.dependentPass);
                _indegree[dep.dependentPass] += 1;
            }
        }

        /**
         * @brief Collects passes that can run immediately and sorts them by registration order.
         * @param _passes All passes in registration order.
         * @param _indegree Current indegree map.
         * @param _passOrder Registration index used as deterministic tie-breaker.
         * @return Ready queue as an ordered vector of zero-indegree passes.
         */
        std::vector<IRenderPass*> BuildInitialReadyPasses(
            const std::vector<IRenderPass*>& _passes,
            const PassInDegreeMap& _indegree,
            const PassOrderMap& _passOrder
        )
        {
            std::vector<IRenderPass*> ready;
            ready.reserve(_passes.size());

            for (IRenderPass* pass : _passes)
            {
                if (_indegree.at(pass) == 0)
                {
                    ready.push_back(pass);
                }
            }

            std::sort(
                ready.begin(),
                ready.end(),
                [&_passOrder](IRenderPass* _a, IRenderPass* _b)
                {
                    return _passOrder.at(_a) < _passOrder.at(_b);
                }
            );

            return ready;
        }

        /**
         * @brief Inserts one ready pass while keeping ready list ordered.
         * @param _ready Current ready passes sorted by registration order.
         * @param _pass Newly ready pass to insert.
         * @param _passOrder Registration index map used for ordering.
         */
        void InsertReadyPass(
            std::vector<IRenderPass*>& _ready,
            IRenderPass* _pass,
            const PassOrderMap& _passOrder
        )
        {
            const auto insertionIt = std::lower_bound(
                _ready.begin(),
                _ready.end(),
                _pass,
                [&_passOrder](IRenderPass* _lhs, IRenderPass* _rhs)
                {
                    return _passOrder.at(_lhs) < _passOrder.at(_rhs);
                }
            );

            _ready.insert(insertionIt, _pass);
        }
    }

    void PassDependencyManager::AddDependency(IRenderPass* _dependent, IRenderPass* _dependsOn)
    {
        CP_EXPECT_MSG(_dependent != nullptr, "Dependent pass is null");
        CP_EXPECT_MSG(_dependsOn != nullptr, "DependsOn pass is null");
        CP_EXPECT_MSG(_dependent != _dependsOn, "Pass cannot depend on itself");

        const auto duplicateIt = std::find_if(
            dependencies.begin(),
            dependencies.end(),
            [_dependent, _dependsOn](const PassDependency& _dep)
            {
                return _dep.dependentPass == _dependent && _dep.dependsOn == _dependsOn;
            }
        );

        if (duplicateIt != dependencies.end())
        {
            return;
        }

        // Check for cycles
        CP_ENSURE_MSG(
            !HasDependencyPath(_dependsOn, _dependent),
            "Adding this dependency would create a cycle"
        );

        dependencies.push_back({ _dependent, _dependsOn });
        RebuildCaches();
    }

    std::vector<IRenderPass*> PassDependencyManager::GetDependencies(IRenderPass* _pass) const
    {
        const auto it = dependenciesCache.find(_pass);
        if (it != dependenciesCache.end())
        {
            return it->second;
        }
        return {};
    }

    std::vector<IRenderPass*> PassDependencyManager::GetDependents(IRenderPass* _pass) const
    {
        const auto it = dependentsCache.find(_pass);
        if (it != dependentsCache.end())
        {
            return it->second;
        }
        return {};
    }

    bool PassDependencyManager::HasDependencyPath(IRenderPass* _from, const IRenderPass* _to) const
    {
        if (_from == _to)
        {
            return true;
        }

        // BFS to check for path
        std::queue<IRenderPass*> queue;
        std::unordered_set<IRenderPass*> visited;

        queue.push(_from);
        visited.insert(_from);

        while (!queue.empty())
        {
            IRenderPass* current = queue.front();
            queue.pop();

            const auto deps = GetDependencies(current);
            for (IRenderPass* dep : deps)
            {
                if (dep == _to)
                {
                    return true;
                }

                if (!visited.contains(dep))
                {
                    visited.insert(dep);
                    queue.push(dep);
                }
            }
        }

        return false;
    }

    std::vector<IRenderPass*> PassDependencyManager::BuildExecutionOrder(const std::vector<IRenderPass*>& _passes) const
    {
        const PassOrderMap passOrder = BuildPassOrderMap(_passes);

        PassInDegreeMap indegree;
        PassAdjacencyMap adjacency;
        InitializeGraphNodes(_passes, indegree, adjacency);
        AddGraphEdgesFromDependencies(dependencies, passOrder, indegree, adjacency);

        std::vector<IRenderPass*> ready = BuildInitialReadyPasses(_passes, indegree, passOrder);

        std::vector<IRenderPass*> result;
        result.reserve(_passes.size());

        while (!ready.empty())
        {
            IRenderPass* current = ready.front();
            ready.erase(ready.begin());
            result.push_back(current);

            for (IRenderPass* dependent : adjacency[current])
            {
                indegree[dependent] -= 1;
                if (indegree[dependent] == 0)
                {
                    InsertReadyPass(ready, dependent, passOrder);
                }
            }
        }

        CP_EXPECT_MSG(
            result.size() == _passes.size(),
            "Cyclic pass dependencies detected while building execution order"
        );

        return result;
    }

    void PassDependencyManager::Clear()
    {
        dependencies.clear();
        dependenciesCache.clear();
        dependentsCache.clear();
    }

    void PassDependencyManager::RebuildCaches()
    {
        dependenciesCache.clear();
        dependentsCache.clear();

        for (const auto& dep : dependencies)
        {
            dependenciesCache[dep.dependentPass].push_back(dep.dependsOn);
            dependentsCache[dep.dependsOn].push_back(dep.dependentPass);
        }
    }
}

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace cp
{
    class IRenderPass;

    /**
     * @brief Pass dependency for scheduling and synchronization.
     * 
     * Defines an execution dependency between render passes.
     * A pass cannot execute until all its dependencies have completed.
     */
    struct PassDependency
    {
        IRenderPass* dependentPass;  // The pass that depends on another
        IRenderPass* dependsOn;      // The pass it waits for
    };

    /**
     * @brief Manager for render pass execution dependencies.
     * 
     * Tracks which passes must execute before others, enabling:
     * - Correct execution ordering
     * - Automatic barrier insertion
     */
    class PassDependencyManager
    {
    public:
        PassDependencyManager() = default;
        ~PassDependencyManager() = default;

        /**
         * @brief Declare that one pass depends on another.
         * @param _dependent The pass that must wait
         * @param _dependsOn The pass that must complete first
         * 
         * Example: PostProcessPass depends on GeometryPass
         * → GeometryPass executes before PostProcessPass
         */
        void AddDependency(IRenderPass* _dependent, IRenderPass* _dependsOn);

        /**
         * @brief Get all passes that a given pass depends on.
         */
        [[nodiscard]] std::vector<IRenderPass*> GetDependencies(IRenderPass* _pass) const;

        /**
         * @brief Get all passes that depend on a given pass.
         */
        [[nodiscard]] std::vector<IRenderPass*> GetDependents(IRenderPass* _pass) const;

        /**
         * @brief Check if there is a dependency path from passA to passB.
         */
        [[nodiscard]] bool HasDependencyPath(IRenderPass* _from, const IRenderPass* _to) const;

        /**
         * @brief Build a topological execution order from declared dependencies.
         * @param _passes All passes that must appear in the final order
         */
        [[nodiscard]] std::vector<IRenderPass*> BuildExecutionOrder(const std::vector<IRenderPass*>& _passes) const;

        /**
         * @brief Clear all dependencies.
         */
        void Clear();

        /**
         * @brief Get all dependencies.
         */
        [[nodiscard]] const std::vector<PassDependency>& GetAllDependencies() const
        {
            return dependencies;
        }

    private:
        std::vector<PassDependency> dependencies;

        // Cached lookups for performance
        std::unordered_map<IRenderPass*, std::vector<IRenderPass*>> dependenciesCache;
        std::unordered_map<IRenderPass*, std::vector<IRenderPass*>> dependentsCache;

        void RebuildCaches();
    };
}

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <utility>

#include "Renderpass.hpp"
#include "FrameGraphBuilder.hpp"
#include "BarrierManager.hpp"
#include "PassDependencyManager.hpp"

namespace cp
{
    class RenderingHardwareInterface;
    class FramegraphResource;
    class FramegraphResourceHandle;

    inline constinit auto SWAPCHAIN_IMAGE_PASS_FINAL_TEXTURE = "SwapchainOutput";

    /**
     * @brief Main framegraph orchestrator.
     * 
     * Manages render passes and their resources through a compile-execute-reset cycle:
     * 
     * 1. CONSTRUCTION: Add passes via AddPass() and equivalents
     * 2. COMPILATION: Compile() sets up resources and allocates GPU memory
     * 3. EXECUTION: Execute() runs all passes (called every frame)
     * 4. RESET: Reset() frees resources and returns to uncompiled state
     * 
     * The framegraph produces a SWAPCHAIN_IMAGE_PASS_FINAL_TEXTURE texture that should be
     * blitted to the swapchain image after execution.
     * 
     * Example usage:
     * @code
     * // Setup (once)
     * FrameGraph frameGraph;
     * frameGraph.AddPass(std::make_unique<GeometryPass>());
     * frameGraph.AddPass(std::make_unique<PostProcessPass>());
     * frameGraph.Compile(rhi);
     * 
     * // Per frame
     * frameGraph.Execute(frameContext);
     * BlitToSwapchain(frameGraph.GetFinalRendering(), swapchainImage);
     * 
     * // On resize
     * frameGraph.Reset();
     * // Re-add passes with new dimensions
     * frameGraph.Compile(rhi);
     * @endcode
     */
    class FrameGraph
    {
    public:
        FrameGraph() = default;
        ~FrameGraph() = default;

        // Prevent copying and moving
        FrameGraph(const FrameGraph&) = delete;
        FrameGraph& operator=(const FrameGraph&) = delete;
        FrameGraph(FrameGraph&&) = delete;
        FrameGraph& operator=(FrameGraph&&) = delete;

        /**
         * @brief Add a render pass to the framegraph.
         * @param _pass The render pass to add (ownership is transferred)
         * 
         * Must be called before Compile().
         */
        void AddPass(std::unique_ptr<IRenderPass> _pass);

        /**
         * @brief Add a typed render pass and return a pointer to it.
         * @param _pass The render pass to add (ownership is transferred)
         * @return Pointer to the added pass (valid until Reset() is called)
         */
        template<typename PassType>
        PassType* AddPassTyped(std::unique_ptr<PassType> _pass)
        {
            PassType* ptr = _pass.get();
            AddPass(std::move(_pass));
            return ptr;
        }

        /**
         * @brief Instantiate and add a typed render pass, then return a pointer to it.
         * @return Pointer to the added pass (valid until Reset() is called)
         */
        template<typename PassType>
        PassType* AddPassTyped()
        {
            std::unique_ptr<PassType> pass = std::make_unique<PassType>();
            PassType* ptr = AddPassTyped<PassType>(pass);
            return ptr;
        }

        /**
         * @brief Declare a dependency between two passes.
         * @param _dependent The pass that must wait (executes second)
         * @param _dependsOn The pass that must complete first
         * 
         * Example:
         * @code
         * auto* geometryPass = frameGraph.AddPassTyped(...);
         * auto* postProcessPass = frameGraph.AddPassTyped(...);
         * 
         * // PostProcess must wait for Geometry to complete
         * frameGraph.AddPassDependency(postProcessPass, geometryPass);
         * @endcode
         * 
         * Ensures correct execution order and enables automatic synchronization.
         * Detects and prevents circular dependencies.
         */
        void AddPassDependency(IRenderPass* _dependent, IRenderPass* _dependsOn);

        /**
         * @brief Declare a dependency by pass names.
         * @return true if both passes exist and dependency was added, false otherwise
         *
         * This helper is optional-safe for modular pipelines where a pass may be absent.
         */
        bool AddPassDependencyIfPresent(std::string_view _dependentPassName, std::string_view _dependsOnPassName);

        /**
         * @brief Compile the framegraph.
         * 
         * Performs the following steps:
         * 1. Setup: Calls Setup() on all passes to declare resources
         * 2. PreCompile: Calls OnPreCompile() on all passes for runtime adjustments
         * 3. Allocation: Allocates GPU memory for all transient resources
         * 4. PostCompile: Calls OnPostCompile() on all passes
         * 
         * After compilation, the framegraph is ready for execution.
         * Can only be called once (call Reset() first to recompile).
         * 
         * @param _rhi The rendering hardware interface for resource allocation
         */
        void Compile(RenderingHardwareInterface& _rhi);

        /**
         * @brief Reset the framegraph to uncompiled state.
         * 
         * - Calls OnReset() on all passes
         * - Frees all transient resources
         * - Clears all passes and resources
         * 
         * After reset, you can add new passes and Compile() again.
         */
        void Reset();

        /**
         * @brief Clear resources and prepare for recompilation without removing passes.
         * 
         * - Calls OnReset() on all passes
         * - Frees all transient resources
         * - Clears the resource builder
         * - Keeps the passes intact
         * 
         * Useful for resize or settings changes where the pass structure stays the same.
         * After calling this, you must call Compile() again.
         */
        void ClearResources();

        /**
         * @brief Execute all render passes.
         * @param _frameContext The frame context containing command buffers
         * 
         * Must be called after Compile().
         * Should be called once per frame.
         * 
         * Executes all passes in order, using command buffers from the frame context.
         */
        void Execute(FrameContext& _frameContext) const;

        /**
         * @brief Get the final rendering texture produced by the framegraph.
         * @return Pointer to the final rendering texture, or nullptr if not compiled
         * 
         * This texture should be blitted to the swapchain image after Execute().
         */
        [[nodiscard]] ITexture* GetFinalRendering() const;

        /**
         * @brief Get a resource by name.
         * @param _name Resource name
         * @return Pointer to the resource, or nullptr if not found
         * 
         * Useful for debugging or accessing intermediate results.
         */
        [[nodiscard]] FramegraphResource* GetResource(const std::string& _name);

        /**
         * @brief Share an existing CPU-side resource within this framegraph.
         */
        template<typename T>
        void ShareCpuResource(const std::string& _name, std::shared_ptr<T> _resource)
        {
            builder.ShareCpuResource(_name, std::move(_resource));
        }

        /**
         * @brief Construct and share a CPU-side resource within this framegraph.
         */
        template<typename T, typename... Args>
        std::shared_ptr<T> EmplaceCpuResource(const std::string& _name, Args&&... _args)
        {
            return builder.EmplaceCpuResource<T>(_name, std::forward<Args>(_args)...);
        }

        /**
         * @brief Retrieve a shared CPU-side resource by name.
         */
        template<typename T>
        [[nodiscard]] std::shared_ptr<T> GetCpuResource(const std::string& _name) const
        {
            return builder.GetCpuResource<T>(_name);
        }

        /**
         * @brief Check if a CPU-side resource exists.
         */
        [[nodiscard]] bool HasCpuResource(const std::string& _name) const
        {
            return builder.HasCpuResource(_name);
        }

        /**
         * @brief Remove a shared CPU-side resource.
         */
        void RemoveCpuResource(const std::string& _name)
        {
            builder.RemoveCpuResource(_name);
        }

        /**
         * @brief Check if the framegraph is compiled and ready for execution.
         */
        [[nodiscard]] bool IsCompiled() const { return isCompiled; }

        /**
         * @brief Get the number of passes in the framegraph.
         */
        [[nodiscard]] size_t GetPassCount() const { return passes.size(); }

        /**
         * @brief Get a render pass by index (for debugging).
         * @param _index Pass index
         * @return Pointer to the pass, or nullptr if index out of bounds
         */
        [[nodiscard]] IRenderPass* GetPass(size_t _index) const;

        /**
         * @brief Test pass presence within the frame graph
         * @param _pass Pass pointer to test
         * @return true if the frame graph contains the given pass, false otherwise
         */
        [[nodiscard]] bool ContainsPass(const IRenderPass* _pass) const;

        /**
         * @brief Get a pass pointer from this frame graph given a name
         * @param _name Pass name
         * @return A pointer of the named pass, nullptr if the frame graph does not contain this pass
         */
        [[nodiscard]] IRenderPass* FindPassByName(std::string_view _name) const;

    private:
        /**
         * @brief Phase 1: Call Setup() on all passes.
         */
        void SetupPhase();

        /**
         * @brief Generate pass dependencies from declared resource accesses.
         */
        void GenerateResourceDependencies();

        /**
         * @brief Build a topological execution order from all dependencies.
         */
        void BuildExecutionOrder();

        /**
         * @brief Phase 2: Call OnPreCompile() on all passes.
         */
        void PreCompilePhase();

        /**
         * @brief Phase 3: Allocate GPU memory for all transient resources.
         */
        void AllocationPhase(RenderingHardwareInterface& _rhi);

        /**
         * @brief Phase 4: Call OnPostCompile() on all passes.
         */
        void PostCompilePhase();

        /**
         * @brief Find the "FinalRendering" resource in the builder.
         */
        void FindFinalRenderingResource();

    private:
        std::vector<std::unique_ptr<IRenderPass>> passes;
        std::vector<PassDependency> explicitDependencies;
        std::vector<IRenderPass*> executionOrder;
        FrameGraphBuilder builder;
        BarrierManager barrierManager;
        PassDependencyManager dependencyManager;
        
        bool isCompiled = false;

        // Cached pointer to the final rendering resource
        FramegraphResource* finalRenderingResource = nullptr;
    };
}

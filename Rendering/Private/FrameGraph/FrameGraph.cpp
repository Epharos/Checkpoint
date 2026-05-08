#include "FrameGraph.hpp"

#include <Common/Core/Assert.hpp>
#include <RHI/RenderingHardwareInterface.hpp>
#include <RHI/Core.hpp>

#include "../Renderer.hpp"
#include "FramegraphResource.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace cp
{
    namespace
    {
        using UsageByPass = std::unordered_map<IRenderPass*, ResourceUsage>;
        using UsageByResource = std::unordered_map<FramegraphResource*, UsageByPass>;
        using PassIndexMap = std::unordered_map<IRenderPass*, size_t>;

        /**
         * @brief Builds the initial execution index for each registered pass.
         * @param _passes Passes in frame-graph registration order.
         * @return A map from pass pointer to its original registration index.
         */
        PassIndexMap BuildPassIndexMap(const std::vector<std::unique_ptr<IRenderPass>>& _passes)
        {
            PassIndexMap passIndex;
            passIndex.reserve(_passes.size());

            for (size_t i = 0; i < _passes.size(); ++i)
            {
                passIndex[_passes[i].get()] = i;
            }

            return passIndex;
        }

        /**
         * @brief Aggregates read/write usage per resource and per pass.
         * @tparam ResourceAccessType Access record type (texture or buffer access).
         * @param _accesses All accesses collected by the frame-graph builder.
         * @return A two-level map: resource -> pass -> combined usage flags.
         */
        template<typename ResourceAccessType>
        UsageByResource BuildUsageByResource(const std::vector<ResourceAccessType>& _accesses)
        {
            UsageByResource usageByResource;

            for (const auto& access : _accesses)
            {
                auto& usage = usageByResource[access.resource][access.pass];
                if (access.isWrite)
                {
                    usage |= ResourceUsage::WriteOnly;
                }
                else
                {
                    usage |= ResourceUsage::ReadOnly;
                }
            }

            return usageByResource;
        }

        /**
         * @brief Returns passes that touch one resource, sorted by registration order.
         * @param _usageByPass Usage flags indexed by pass for a single resource.
         * @param _passIndex Registration order index of all passes.
         * @return Ordered list of passes with non-empty usage for the resource.
         */
        std::vector<IRenderPass*> BuildOrderedPasses(
            const UsageByPass& _usageByPass,
            const PassIndexMap& _passIndex
        )
        {
            std::vector<IRenderPass*> orderedPasses;
            orderedPasses.reserve(_usageByPass.size());

            for (const auto& [pass, usage] : _usageByPass)
            {
                if (usage != ResourceUsage::None)
                {
                    orderedPasses.push_back(pass);
                }
            }

            std::sort(
                orderedPasses.begin(),
                orderedPasses.end(),
                [&_passIndex](IRenderPass* _a, IRenderPass* _b)
                {
                    return _passIndex.at(_a) < _passIndex.at(_b);
                }
            );

            return orderedPasses;
        }

        /**
         * @brief Adds dependencies for hazards affecting one specific resource.
         *
         * A dependency is created from later pass to earlier pass whenever either side writes
         * the resource (read-after-write, write-after-read, write-after-write hazards).
         * 
         * @param _usageByPass Usage flags indexed by pass for the resource.
         * @param _orderedPasses Passes using the resource, sorted by execution order.
         * @param _dependencyManager Dependency graph to update.
         */
        void AddHazardDependenciesForResource(
            const UsageByPass& _usageByPass,
            const std::vector<IRenderPass*>& _orderedPasses,
            PassDependencyManager& _dependencyManager
        )
        {
            for (size_t i = 0; i < _orderedPasses.size(); ++i)
            {
                IRenderPass* earlierPass = _orderedPasses[i];
                const ResourceUsage& earlierUsage = _usageByPass.at(earlierPass);

                for (size_t j = i + 1; j < _orderedPasses.size(); ++j)
                {
                    IRenderPass* laterPass = _orderedPasses[j];
                    const ResourceUsage& laterUsage = _usageByPass.at(laterPass);

                    const bool hasWriteHazard =
                        (earlierUsage & ResourceUsage::WriteOnly) != ResourceUsage::None
                        || (laterUsage & ResourceUsage::WriteOnly) != ResourceUsage::None;
                    if (!hasWriteHazard)
                    {
                        continue;
                    }

                    if (_dependencyManager.HasDependencyPath(earlierPass, laterPass))
                    {
                        continue;
                    }

                    _dependencyManager.AddDependency(laterPass, earlierPass);
                }
            }
        }

        /**
         * @brief Infers and adds pass dependencies from resource access declarations.
         * @tparam ResourceAccessType Access record type (texture or buffer access).
         * @param _accesses All accesses for one resource category.
         * @param _passes Registered frame-graph passes.
         * @param _dependencyManager Dependency graph to update with inferred edges.
         */
        template<typename ResourceAccessType>
        void AddDependenciesForAccesses(
            const std::vector<ResourceAccessType>& _accesses,
            const std::vector<std::unique_ptr<IRenderPass>>& _passes,
            PassDependencyManager& _dependencyManager
        )
        {
            const PassIndexMap passIndex = BuildPassIndexMap(_passes);
            const UsageByResource usageByResource = BuildUsageByResource(_accesses);

            for (const auto& [_, usageByPass] : usageByResource)
            {
                const std::vector<IRenderPass*> orderedPasses = BuildOrderedPasses(usageByPass, passIndex);
                AddHazardDependenciesForResource(usageByPass, orderedPasses, _dependencyManager);
            }
        }
    }

    void FrameGraph::AddPass(std::unique_ptr<IRenderPass> _pass)
    {
        CP_EXPECT_MSG(_pass != nullptr, "Cannot add null pass");
        CP_EXPECT_MSG(!isCompiled, "Cannot add pass after compilation. Call Reset() first.");

        passes.push_back(std::move(_pass));
        
        CP_ENSURE_MSG(!passes.empty(), "Pass was not added to the framegraph");
    }

    void FrameGraph::AddPassDependency(IRenderPass* _dependent, IRenderPass* _dependsOn)
    {
        CP_EXPECT_MSG(!isCompiled, "Cannot add dependencies after compilation. Call ClearResources() first.");
        CP_EXPECT_MSG(_dependent != nullptr, "Dependent pass is null");
        CP_EXPECT_MSG(_dependsOn != nullptr, "DependsOn pass is null");
        CP_EXPECT_MSG(ContainsPass(_dependent), "Dependent pass does not belong to this FrameGraph");
        CP_EXPECT_MSG(ContainsPass(_dependsOn), "DependsOn pass does not belong to this FrameGraph");

        explicitDependencies.push_back({ _dependent, _dependsOn });
    }

    bool FrameGraph::AddPassDependencyIfPresent(
        const std::string_view _dependentPassName,
        const std::string_view _dependsOnPassName
    )
    {
        CP_EXPECT_MSG(!isCompiled, "Cannot add dependencies after compilation. Call ClearResources() first.");

        IRenderPass* dependent = FindPassByName(_dependentPassName);
        IRenderPass* dependsOn = FindPassByName(_dependsOnPassName);

        if (dependent == nullptr || dependsOn == nullptr)
        {
            return false;
        }

        AddPassDependency(dependent, dependsOn);
        return true;
    }

    void FrameGraph::Compile(RenderingHardwareInterface& _rhi)
    {
        CP_EXPECT_MSG(!isCompiled, "FrameGraph already compiled. Call Reset() first.");
        CP_EXPECT_MSG(!passes.empty(), "Cannot compile empty framegraph. Add passes first.");

        barrierManager.Clear();

        // Phase 1: DeclareDependencies - Passes register explicit execution ordering
        DeclarePassDependenciesPhase();

        // Phase 2: DeclareResources - All passes register their created/imported resources
        DeclareResourcesPhase();

        // Phase 3: Setup - Passes declare how they use the now-existing resources
        SetupPhase();

        // Build dependency graph from explicit + inferred resource dependencies
        dependencyManager.Clear();
        for (const auto& dep : explicitDependencies)
        {
            dependencyManager.AddDependency(dep.dependentPass, dep.dependsOn);
        }
        GenerateResourceDependencies();
        BuildExecutionOrder();

        // Phase 2: PreCompile - Runtime adjustments
        PreCompilePhase();

        // Phase 3: Allocation - Create GPU resources for transient resources
        AllocationPhase(_rhi);

        // Build per-pass barriers from declared resource synchronization
        barrierManager.BuildPassBarriers(
            executionOrder,
            builder.GetTextureAccesses(),
            builder.GetBufferAccesses(),
            builder.GetTextureFinalStates(),
            builder.GetBufferFinalStates()
        );

        // Phase 4: PostCompile - Passes can finalize setup
        PostCompilePhase();

        // Cache the final rendering resource
        FindFinalRenderingResource();

        isCompiled = true;
        
        CP_ENSURE_MSG(isCompiled, "FrameGraph compilation failed");
    }

    void FrameGraph::Reset()
    {
        // Call OnReset on all passes
        for (const auto& pass : passes)
        {
            pass->OnReset();
        }

        // Free transient resources
        for (const auto& resource : builder.GetResources())
        {
            resource->Reset();
        }

        // Clear everything
        passes.clear();
        explicitDependencies.clear();
        executionOrder.clear();
        builder.Clear();
        barrierManager.Clear();
        dependencyManager.Clear();
        finalRenderingResource = nullptr;
        isCompiled = false;
    }

    void FrameGraph::ClearResources()
    {
        // Call OnReset on all passes
        for (const auto& pass : passes)
        {
            pass->OnReset();
        }

        // Free transient resources
        for (const auto& resource : builder.GetResources())
        {
            resource->Reset();
        }

        // Clear only resources, keep passes and dependencies
        builder.Clear();
        executionOrder.clear();
        barrierManager.Clear();
        finalRenderingResource = nullptr;
        isCompiled = false;
    }

    void FrameGraph::Execute(FrameContext& _frameContext) const
    {
        if (!isCompiled) return;

        // Execute each pass in scheduled order
        CP_EXPECT_MSG(
            executionOrder.size() == passes.size(),
            "Execution order is invalid. Are you sure you compiled the framegraph?"
        );

        for (size_t i = 0; i < executionOrder.size(); ++i)
        {
            // Get the primary command buffer for this pass
            // For now, all passes use graphics queue
            ICommandBuffer& primaryCmd = _frameContext.GetCommandBuffer(QueueType::Graphics, static_cast<uint32_t>(i));

            primaryCmd.Begin();

            for (const IBarrier& barrier : barrierManager.GetPassBeginBarriers(executionOrder[i]))
            {
                primaryCmd.AddBarrier(barrier);
            }

            // Create execution context
            RenderPassExecutionContext executionContext
            {
                .primaryCmdBuffer = primaryCmd,
                .frameContext = _frameContext
            };

            // Execute the pass
            CP_ASSERT_MSG(executionOrder[i] != nullptr, "Pass is null during execution");
            executionOrder[i]->Execute(executionContext);

            for (const IBarrier& barrier : barrierManager.GetPassEndBarriers(executionOrder[i]))
            {
                primaryCmd.AddBarrier(barrier);
            }

            primaryCmd.End();
        }
    }

    ITexture* FrameGraph::GetFinalRendering() const
    {
        if (!isCompiled || !finalRenderingResource)
        {
            return nullptr;
        }

        return finalRenderingResource->GetTexture();
    }

    FramegraphResource* FrameGraph::GetResource(const std::string& _name)
    {
        return builder.GetResourceByName(_name);
    }

    IRenderPass* FrameGraph::GetPass(const size_t _index) const
    {
        if (_index >= passes.size())
        {
            return nullptr;
        }

        return passes[_index].get();
    }

    void FrameGraph::DeclarePassDependenciesPhase()
    {
        for (const auto& pass : passes)
        {
            pass->DeclareDependencies(*this);
        }
    }

    void FrameGraph::DeclareResourcesPhase()
    {
        for (const auto& pass : passes)
        {
            builder.BeginPassSetup(pass.get());
            pass->DeclareResources(builder);
            builder.EndPassSetup();
        }
    }

    void FrameGraph::SetupPhase()
    {
        for (const auto& pass : passes)
        {
            builder.BeginPassSetup(pass.get());
            pass->Setup(builder);
            builder.EndPassSetup();
        }
    }

    void FrameGraph::GenerateResourceDependencies()
    {
        AddDependenciesForAccesses(builder.GetTextureAccesses(), passes, dependencyManager);
        AddDependenciesForAccesses(builder.GetBufferAccesses(), passes, dependencyManager);
    }

    void FrameGraph::BuildExecutionOrder()
    {
        std::vector<IRenderPass*> passPointers;
        passPointers.reserve(passes.size());

        for (const auto& pass : passes)
        {
            passPointers.push_back(pass.get());
        }

        executionOrder = dependencyManager.BuildExecutionOrder(passPointers);
    }

    bool FrameGraph::ContainsPass(const IRenderPass* _pass) const
    {
        if (_pass == nullptr)
        {
            return false;
        }

        return std::any_of(
            passes.begin(),
            passes.end(),
            [_pass](const std::unique_ptr<IRenderPass>& _candidate)
            {
                return _candidate.get() == _pass;
            }
        );
    }

    IRenderPass* FrameGraph::FindPassByName(const std::string_view _name) const
    {
        for (const auto& pass : passes)
        {
            if (pass->GetName() == _name)
            {
                return pass.get();
            }
        }

        return nullptr;
    }

    void FrameGraph::PreCompilePhase()
    {
        // Call OnPreCompile() on each pass
        // Passes can make runtime adjustments here
        for (const auto& pass : passes)
        {
            pass->OnPreCompile(*this);
        }
    }

    void FrameGraph::AllocationPhase(RenderingHardwareInterface& _rhi)
    {
        // Allocate GPU resources for all transient resources
        for (const auto& resource : builder.GetResources())
        {
            // Skip non-transient resources (imported/external)
            if (!resource->IsTransient())
            {
                continue;
            }

            // Skip already allocated resources
            if (!resource->IsVirtual())
            {
                continue;
            }

            // Allocate based on resource type
            if (resource->IsTexture())
            {
                const TextureInfo& info = resource->GetTextureInfo();
                const auto texture = _rhi.CreateTexture(info);
                resource->AllocateTexture(texture);
            }
            else if (resource->IsBuffer())
            {
                const BufferInfo& info = resource->GetBufferInfo();
                const auto buffer = _rhi.CreateBuffer(info);
                resource->AllocateBuffer(buffer);
            }
        }
    }

    void FrameGraph::PostCompilePhase()
    {
        // Call OnPostCompile() on each pass
        // Passes can access allocated resources here
        for (const auto& pass : passes)
        {
            pass->OnPostCompile(*this);
        }
    }

    void FrameGraph::FindFinalRenderingResource()
    {
        // Try to find a resource named "FinalRendering"
        finalRenderingResource = builder.GetResourceByName("FinalRendering");

        // Warning if not found (optional, can be removed if not always needed)
        if (!finalRenderingResource)
        {
            // Note: This is not an error, some framegraphs might not produce a final rendering
            // (e.g., compute-only framegraphs)
        }
    }
}

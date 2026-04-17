#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include <Common/Data/Extent.hpp>

namespace cp
{
    class AssetRegistry;
    class FrameGraphBuilder;
    class FrameGraph;
    class ICommandBuffer;
    class RenderingHardwareInterface;
    struct FrameContext;

    struct RenderPassInitContext
    {
        RenderingHardwareInterface& rhi;
        AssetRegistry& assetRegistry;
        Extent2D<uint32_t> renderExtent;
    };

    struct IConfigurableRenderPass
    {
        virtual void Configure(const RenderPassInitContext& _context) = 0;
        virtual ~IConfigurableRenderPass() = default;
    };

    /**
     * @brief Execution context passed to render passes, providing access to command buffers.
     * 
     * Ex: Simple pass using implicit conversion:
     * @code
     * void Execute(RenderPassExecutionContext& ctx) override
     * {
     *     ICommandBuffer& cmd = ctx;  // Implicit conversion to primary buffer
     *     cmd.BeginRendering(...);
     *     cmd.Draw(...);
     *     cmd.EndRendering();
     * }
     * @endcode
     * 
     * Ex: Multi-queue pass using async compute:
     * @code
     * void Execute(RenderPassExecutionContext& ctx) override
     * {
     *     auto& gfx = ctx.GetGraphicsBuffer();
     *     auto& compute = ctx.GetComputeBuffer();
     *     
     *     // Record compute work
     *     compute.Begin();
     *     compute.BindPipeline(*computePipeline);
     *     compute.Dispatch(16, 16, 1);
     *     compute.End();
     *     
     *     // Record graphics work
     *     gfx.Begin();
     *     gfx.BeginRendering(...);
     *     gfx.Draw(...);
     *     gfx.EndRendering();
     *     gfx.End();
     * }
     * @endcode
     */
    struct RenderPassExecutionContext
    {
        ICommandBuffer& primaryCmdBuffer;
        FrameContext& frameContext;

        /**
         * @brief Implicit conversion to primary command buffer for simple passes.
         */
        operator ICommandBuffer&() const { return primaryCmdBuffer; }

        /**
         * @brief Get a graphics command buffer.
         * @param _index Index of the buffer (default: 0)
         */
        [[nodiscard]] ICommandBuffer& GetGraphicsBuffer(uint32_t _index = 0) const;

        /**
         * @brief Get a compute command buffer.
         * @param _index Index of the buffer (default: 0)
         */
        [[nodiscard]] ICommandBuffer& GetComputeBuffer(uint32_t _index = 0) const;

        /**
         * @brief Get a copy/transfer command buffer.
         * @param _index Index of the buffer (default: 0)
         */
        [[nodiscard]] ICommandBuffer& GetCopyBuffer(uint32_t _index = 0) const;
    };

    struct IRenderPass
    {
        /**
         * @brief Called when the pass is added to the framegraph to declare resources used.
         * @param _builder The framegraph builder for resource declaration.
         */
        virtual void Setup(FrameGraphBuilder& _builder) = 0;

        /**
         * @brief Called right before the framegraph gets compiled and allow for pass modifications based on runtime conditions.
         * @param _framegraph The framegraph being compiled.
         */
        virtual void OnPreCompile(FrameGraph& _framegraph) = 0;

        /**
         * @brief Called right after the framegraph got compiled and resources were allocated.
         * @param _framegraph The framegraph being compiled.
         */
        virtual void OnPostCompile(FrameGraph& _framegraph) = 0;

        /**
         * @brief Called when the framegraph is reset to clean up pass resources.
         */
        virtual void OnReset() = 0;

        /**
         * @brief Executes the pass.
         * @param _context Execution context providing access to command buffers and frame resources.
         */
        virtual void Execute(RenderPassExecutionContext& _context) = 0;

        /**
         * @return The name of the pass.
         */
        virtual std::string_view GetName() = 0;

        virtual ~IRenderPass() = default;
    };

    template <typename Data>
    class RenderPass : public IRenderPass
    {
    public:
        void Setup(FrameGraphBuilder &_builder) override = 0;
        void OnPreCompile(FrameGraph &_framegraph) override {}
        void OnPostCompile(FrameGraph &_framegraph) override {}
        void Execute(RenderPassExecutionContext& _context) override = 0;
        void OnReset() override {}

        std::string_view GetName() override { return name; }

        Data& GetData() { return data; }
        const Data& GetData() const { return data; }

    protected:
        Data data;

        std::string name;
    };
}

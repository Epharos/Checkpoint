#pragma once

#include <Common/Data/Color.hpp>
#include <Common/Data/Extent.hpp>

namespace cp
{
    class IBarrier;
    class ICommandAllocator;

    enum class CommandBufferType
    {
        Graphics,
        Compute,
        Copy
    };

    enum class IndexType
    {
        UInt8,
        UInt16,
        UInt32,
        None
    };

    enum class LoadOp
    {
        Load,
        Clear,
        DontCare,
        None
    };

    enum class StoreOp
    {
        Store,
        DontCare,
        None
    };

    struct ClearDepthStencil
    {
        float depth;
        uint32_t stencil;
    };

    struct ColorAttachmentInfo
    {
        ITexture* texture;
        uint32_t mipLevel = 0;
        uint32_t arrayLayer = 0;

        LoadOp loadOp = LoadOp::DontCare;
        StoreOp storeOp = StoreOp::DontCare;

        Color clearValue;
    };

    struct DepthStencilAttachmentInfo
    {
        ITexture* texture;
        uint32_t mipLevel = 0;
        uint32_t arrayLayer = 0;

        LoadOp depthLoadOp = LoadOp::DontCare;
        StoreOp depthStoreOp = StoreOp::DontCare;

        LoadOp stencilLoadOp = LoadOp::DontCare;
        StoreOp stencilStoreOp = StoreOp::DontCare;

        ClearDepthStencil clearValue;
    };

    struct RenderingInfo
    {
        Extent2D<uint32_t> extent;
        uint32_t layers = 1;

        std::vector<ColorAttachmentInfo> colorAttachments;
        std::optional<DepthStencilAttachmentInfo> depthStencilAttachment;
    };

    class ICommandBuffer
    {
        struct Viewport;
        struct Rectangle;
        class IPipeline;
        class IDescriptorSet;
        class IBuffer;

    public:
        ICommandBuffer(ICommandAllocator& _commandAllocator) {};
        virtual ~ICommandBuffer() = default;

        virtual CommandBufferType GetType() const = 0;

        /**
        * @brief Begins recording commands into this command buffer.
        *        Must be called before any other recording command.
        */
        virtual void Begin() = 0;

        /**
        * @brief Ends recording commands into this command buffer.
        *        Must be called before submitting the command buffer to a queue.
        */
        virtual void End() = 0;

        /**
        * @brief Records a pipeline barrier into the command buffer.
        *        Used to synchronize execution and memory access between commands, and to perform image layout transitions.
        *
        * @param _barrier The barrier to insert, describing source/destination stages, access masks and layout transitions.
        */
        virtual void AddBarrier(const IBarrier& _barrier) = 0;

        // Rendering

        /**
        * @brief Begins a dynamic rendering pass using the provided attachment descriptions.
        *        Rendering commands (draw calls) must be recorded between BeginRendering and EndRendering.
        *
        * @param _renderingInfo The description of color and depth/stencil attachments, extent and layer count.
        */
        virtual void BeginRendering(const RenderingInfo& _renderingInfo) = 0;

        /**
        * @brief Ends the current dynamic rendering pass.
        */
        virtual void EndRendering() = 0;

        // virtual void SetViewport(const Viewport& _viewport) = 0;
        // virtual void SetScissor(const Rectangle& _rectangle) = 0;
        //
        // virtual void BindPipeline(IPipeline* _pipeline) = 0;
        // virtual void BindDescriptorSet(uint32_t _binding, IDescriptorSet* _set) = 0;
        // virtual void BindVertexBuffer(uint32_t _binding, IBuffer* _vertexBuffer) = 0;
        // virtual void BindIndexBuffer(IBuffer* _indexBuffer, IndexType _indexType) = 0;

        // virtual void Draw(
        //     uint32_t _vertexCount,
        //     uint32_t _instanceCount,
        //     uint32_t _firstVertex,
        //     uint32_t _firstInstance
        // ) = 0;
        //
        // virtual void DrawIndexed(
        //     uint32_t _indexCount,
        //     uint32_t _instanceCount,
        //     uint32_t _firstIndex,
        //     uint32_t _firstVertex,
        //     uint32_t _firstInstance
        // ) = 0;
        //
        // // TODO : Add indirect draw methods
        //
        // // Compute
        //
        // virtual void Dispatch(
        //     uint32_t _groupCountX,
        //     uint32_t _groupCountY,
        //     uint32_t _groupCountZ
        // ) = 0;

        // Copy

        // TODO : Add copy buffers and textures methods
    };
}

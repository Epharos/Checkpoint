#pragma once

#include <optional>

#include <Common/Data/Color.hpp>
#include <Common/Data/Offset.hpp>

namespace cp
{
    enum class Filter : uint8_t;
    class ICommandAllocator;
    class IBarrier;
    class IPipeline;
    class IDescriptorSet;
    class IBuffer;
    class ITexture;

    struct Viewport;
    struct Rectangle2D;

    struct BufferTextureCopyRegion
    {
        uint64_t bufferOffset = 0;
        uint32_t bufferRowLength = 0;
        uint32_t bufferImageHeight = 0;

        uint32_t mipLevel = 0;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;

        Offset3D<int32_t> textureOffset { 0, 0, 0 };
        Extent3D<uint32_t> textureExtent { 0, 0, 0 };
    };

    struct TextureCopyRegion
    {
        uint32_t srcMipLevel = 0;
        uint32_t srcBaseArrayLayer = 0;
        uint32_t srcLayerCount = 1;
        Offset3D<int32_t> srcOffset { 0, 0, 0 };

        uint32_t dstMipLevel = 0;
        uint32_t dstBaseArrayLayer = 0;
        uint32_t dstLayerCount = 1;
        Offset3D<int32_t> dstOffset { 0, 0, 0 };

        Extent3D<uint32_t> extent { 0, 0, 0 };
    };

    struct TextureBlitRegion
    {
        uint32_t srcMipLevel = 0;
        uint32_t srcBaseArrayLayer = 0;
        uint32_t srcLayerCount = 1;
        Offset3D<int32_t> srcOffsets[2] { Offset3D{ 0, 0, 0 }, Offset3D{ 0, 0, 0 } };

        uint32_t dstMipLevel = 0;
        uint32_t dstBaseArrayLayer = 0;
        uint32_t dstLayerCount = 1;
        Offset3D<int32_t> dstOffsets[2] { Offset3D{ 0, 0, 0 }, Offset3D{ 0, 0, 0 } };
    };

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
        ITexture* texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t arrayLayer = 0;

        LoadOp loadOp = LoadOp::DontCare;
        StoreOp storeOp = StoreOp::DontCare;

        Color clearValue;
    };

    struct DepthStencilAttachmentInfo
    {
        ITexture* texture = nullptr;
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


    public:
        ICommandBuffer(ICommandAllocator& _commandAllocator) {};
        virtual ~ICommandBuffer() = default;

        [[nodiscard]] virtual CommandBufferType GetType() const = 0;

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

        virtual void SetViewport(const Viewport& _viewport) = 0;
        virtual void SetScissor(const Rectangle2D &_rectangle) = 0;

        virtual void BindPipeline(IPipeline& _pipeline) = 0;
        virtual void BindDescriptorSet(uint32_t _binding, IDescriptorSet& _set) = 0;
        virtual void BindVertexBuffer(uint32_t _binding, IBuffer& _vertexBuffer) = 0;
        virtual void BindIndexBuffer(IBuffer& _indexBuffer, IndexType _indexType) = 0;

        virtual void Draw(
            uint32_t _vertexCount,
            uint32_t _instanceCount,
            uint32_t _firstVertex,
            uint32_t _firstInstance
        ) = 0;

        // virtual void DrawIndexed(
        //     uint32_t _indexCount,
        //     uint32_t _instanceCount,
        //     uint32_t _firstIndex,
        //     uint32_t _firstVertex,
        //     uint32_t _firstInstance
        // ) = 0;
        //
        // TODO : Add indirect draw methods

        // Compute

        virtual void Dispatch(
            uint32_t _groupCountX,
            uint32_t _groupCountY,
            uint32_t _groupCountZ
        ) = 0;

        // Copy

        /**
        * @brief Copies data from a buffer to a texture.
        *        The texture must be in TransferDst layout and the buffer must have TransferSrc usage.
        *
        * @param _srcBuffer Source buffer containing pixel data
        * @param _dstTexture Destination texture
        * @param _region Description of the copy region (offset, extent, mip level, array layer)
        */
        virtual void CopyBufferToTexture(
            IBuffer& _srcBuffer,
            ITexture& _dstTexture,
            const BufferTextureCopyRegion& _region
        ) = 0;

        /**
        * @brief Copies image data from one texture to another.
        *        Both textures must have compatible formats.
        *        Source texture must be in TransferSrc layout, destination in TransferDst layout.
        *
        * @param _srcTexture Source texture
        * @param _dstTexture Destination texture
        * @param _region Description of the copy region (offsets, extents, mip levels, array layers)
        */
        virtual void CopyTexture(
            ITexture& _srcTexture,
            ITexture& _dstTexture,
            const TextureCopyRegion& _region
        ) = 0;

        /**
        * @brief Blits (copies with potential scaling and filtering) from one texture to another.
        *        Can perform scaling, format conversion, and filtering.
        *        Source texture must be in TransferSrc layout, destination in TransferDst layout.
        *
        * @param _srcTexture Source texture
        * @param _dstTexture Destination texture
        * @param _region Description of the blit region (source and destination offsets for scaling)
        * @param _filter Filter to use for scaling (Nearest or Linear)
        */
        virtual void BlitTexture(
            ITexture& _srcTexture,
            ITexture& _dstTexture,
            const TextureBlitRegion& _region,
            Filter _filter
        ) = 0;
    };
}

#pragma once

#include <RHI/Core.hpp>

namespace cp
{
    class CommandAllocator;
    class Device;
    class Pipeline;

    struct Viewport;

    class CommandBuffer final : public ICommandBuffer
    {
    public:
        CommandBuffer(ICommandAllocator& _commandAllocator);
        ~CommandBuffer() override;

    public:
        [[nodiscard]] vk::CommandBuffer& GetHandle() { return commandBuffer; }
        [[nodiscard]] const vk::CommandBuffer& GetHandle() const { return commandBuffer; }

    public:
        void Begin() override;
        void End() override;

        void AddBarrier(const IBarrier &_barrier) override;

        [[nodiscard]] CommandBufferType GetType() const override;

        void BeginRendering(const RenderingInfo& _internalRenderingInfo) override;
        void EndRendering() override;

        void SetViewport(const Viewport& _viewport) override;
        void SetScissor(const Rectangle2D& _rectangle) override;

        /**
         * @brief Binds a graphics or compute pipeline to the command buffer.
         *        Subsequent draw or dispatch commands will use this pipeline's state and shaders.
         * 
         * @param _pipeline The pipeline to bind (graphics or compute).
         */
        void BindPipeline(IPipeline& _pipeline) override;

        /**
         * @brief Binds a descriptor set to the command buffer at the specified binding index.
         *        Descriptor sets provide resources (buffers, textures, samplers) to shader stages.
         *        A pipeline must be bound before calling this method.
         * 
         * @param _binding The binding index in the pipeline layout (set number).
         * @param _set The descriptor set containing the resources to bind.
         */
        void BindDescriptorSet(uint32_t _binding, IDescriptorSet& _set) override;

        /**
         * @brief Binds a vertex buffer to the command buffer at the specified binding slot.
         *        The vertex data will be read from this buffer during draw calls.
         * 
         * @param _binding The binding slot for the vertex buffer (must match shader input).
         * @param _vertexBuffer The buffer containing vertex data.
         */
        void BindVertexBuffer(uint32_t _binding, IBuffer& _vertexBuffer) override;

        /**
         * @brief Binds an index buffer to the command buffer.
         *        Indexed draw calls will use this buffer to fetch vertex indices.
         * 
         * @param _indexBuffer The buffer containing index data, or nullptr to unbind.
         * @param _indexType The type of indices in the buffer (UInt16, UInt32, etc.).
         */
        void BindIndexBuffer(IBuffer& _indexBuffer, IndexType _indexType) override;

        /**
         * @brief Records a non-indexed draw command.
         *        Draws primitives using the currently bound vertex buffers and pipeline.
         * 
         * @param _vertexCount Number of vertices to draw.
         * @param _instanceCount Number of instances to draw.
         * @param _firstVertex Index of the first vertex to draw.
         * @param _firstInstance Instance ID of the first instance to draw.
         */
        void Draw(
            uint32_t _vertexCount,
            uint32_t _instanceCount,
            uint32_t _firstVertex,
            uint32_t _firstInstance
        ) override;
        //
        // void DrawIndexed(
        //     uint32_t _indexCount,
        //     uint32_t _instanceCount,
        //     uint32_t _firstIndex,
        //     uint32_t _firstVertex,
        //     uint32_t _firstInstance
        // ) override;

        void Dispatch(uint32_t _groupCountX, uint32_t _groupCountY, uint32_t _groupCountZ) override;

        void CopyBufferToTexture(
            IBuffer& _srcBuffer,
            ITexture& _dstTexture,
            const BufferTextureCopyRegion& _region
        ) override;

        void CopyTexture(
            ITexture& _srcTexture,
            ITexture& _dstTexture,
            const TextureCopyRegion& _region
        ) override;

        void BlitTexture(
            ITexture& _srcTexture,
            ITexture& _dstTexture,
            const TextureBlitRegion& _region,
            Filter _filter
        ) override;

    public:
        void Initialize();
        void Cleanup() const;

    private:
        CommandAllocator& commandAllocator;

        vk::CommandBuffer commandBuffer { VK_NULL_HANDLE };
        
        Pipeline* currentPipeline = nullptr;
    };
}

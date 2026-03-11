#pragma once

#include <RHI/Core.hpp>

namespace cp
{
    class CommandAllocator;
    class Device;

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

        CommandBufferType GetType() const override;

        void BeginRendering(const RenderingInfo& _internalRenderingInfo) override;
        void EndRendering() override;

        // void SetViewport(const Viewport &_viewport) override;
        // void SetScissor(const Rectangle &_rectangle) override;
        //
        // void BindPipeline(IPipeline *_pipeline) override;
        // void BindDescriptorSet(uint32_t _binding, IDescriptorSet *_set) override;
        // void BindVertexBuffer(uint32_t _binding, IBuffer *_vertexBuffer) override;
        // void BindIndexBuffer(IBuffer *_indexBuffer, IndexType _indexType) override;
        //
        // void Draw(
        //     uint32_t _vertexCount,
        //     uint32_t _instanceCount,
        //     uint32_t _firstVertex,
        //     uint32_t _firstInstance
        // ) override;
        //
        // void DrawIndexed(
        //     uint32_t _indexCount,
        //     uint32_t _instanceCount,
        //     uint32_t _firstIndex,
        //     uint32_t _firstVertex,
        //     uint32_t _firstInstance
        // ) override;
        //
        // void Dispatch(uint32_t _groupCountX, uint32_t _groupCountY, uint32_t _groupCountZ) override;

    public:
        void Initialize();
        void Cleanup() const;

    private:
        CommandAllocator& commandAllocator;

        vk::CommandBuffer commandBuffer { VK_NULL_HANDLE };
    };
}

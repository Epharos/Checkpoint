#pragma once

#include "ComponentTypeInfo.hpp"
#include "Entity.hpp"

#include <Common/Core/Assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp::ecs
{
    /**
     * Stores entities that share the exact same component signature.
     * Data is laid out in SoA columns and grouped by fixed-size chunks.
     */
    class Archetype final
    {
    public:
        struct ColumnLayout
        {
            ComponentTypeId componentId = 0;
            size_t componentSize = 0;
            size_t componentAlignment = 1;
            ComponentTypeInfo::MoveConstructFn moveConstruct = nullptr;
            ComponentTypeInfo::DestroyFn destroy = nullptr;
        };

        Archetype(
            std::vector<ComponentTypeId> _signature,
            std::vector<ColumnLayout> _columnLayouts,
            size_t _chunkCapacity
        );
        ~Archetype();

        Archetype(const Archetype&) = delete;
        Archetype& operator=(const Archetype&) = delete;
        Archetype(Archetype&&) = delete;
        Archetype& operator=(Archetype&&) = delete;

        [[nodiscard]] const std::vector<ComponentTypeId>& Signature() const;
        [[nodiscard]] bool Contains(std::span<const ComponentTypeId> _requiredComponents) const;
        [[nodiscard]] bool HasComponent(ComponentTypeId _componentId) const;

        [[nodiscard]] size_t ChunkCount() const;
        [[nodiscard]] size_t ChunkEntityCount(uint32_t _chunkIndex) const;

        [[nodiscard]] Entity EntityAt(uint32_t _chunkIndex, uint32_t _rowIndex) const;
        void SetEntityAt(uint32_t _chunkIndex, uint32_t _rowIndex, Entity _entity);

        void AppendEntity(Entity _entity, uint32_t& _outChunkIndex, uint32_t& _outRowIndex);
        void DecrementChunkCount(uint32_t _chunkIndex);

        [[nodiscard]] void* ComponentPtr(ComponentTypeId _componentId, uint32_t _chunkIndex, uint32_t _rowIndex);
        [[nodiscard]] const void* ComponentPtr(ComponentTypeId _componentId, uint32_t _chunkIndex, uint32_t _rowIndex) const;

        void DestroyComponent(ComponentTypeId _componentId, uint32_t _chunkIndex, uint32_t _rowIndex);
        void MoveComponentTo(
            ComponentTypeId _componentId,
            uint32_t _sourceChunkIndex,
            uint32_t _sourceRowIndex,
            Archetype& _target,
            uint32_t _targetChunkIndex,
            uint32_t _targetRowIndex
        );
        void MoveRowWithinChunk(uint32_t _chunkIndex, uint32_t _sourceRowIndex, uint32_t _targetRowIndex) const;

    private:
        struct Chunk
        {
            size_t count = 0;
            std::vector<Entity> entities;
            std::vector<void*> columns;
        };

        [[nodiscard]] const ColumnLayout& LayoutFor(ComponentTypeId _componentId) const;
        [[nodiscard]] Chunk& EnsureChunkForAppend();
        [[nodiscard]] Chunk CreateChunk() const;
        void DestroyChunk(Chunk& _chunk) const;

    private:
        std::vector<ComponentTypeId> signature;
        std::unordered_map<ComponentTypeId, size_t> columnIndexByComponent;
        std::vector<ColumnLayout> columnLayouts;
        std::vector<Chunk> chunks;
        size_t chunkCapacity = 0;
    };

    struct SignatureHasher
    {
        [[nodiscard]] size_t operator()(const std::vector<ComponentTypeId>& _signature) const noexcept;
    };
}

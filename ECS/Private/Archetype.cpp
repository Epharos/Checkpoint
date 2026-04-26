#include "Archetype.hpp"

namespace cp::ecs
{
    Archetype::Archetype(
        std::vector<ComponentTypeId> _signature,
        std::vector<ColumnLayout> _columnLayouts,
        const size_t _chunkCapacity
    )
        : signature(std::move(_signature))
        , columnLayouts(std::move(_columnLayouts))
        , chunkCapacity(_chunkCapacity)
    {
        CP_EXPECT_MSG(!columnLayouts.empty() || signature.empty(), "Archetype layout/signature mismatch.");
        CP_EXPECT_MSG(chunkCapacity > 0, "Archetype chunk capacity must be > 0.");

        for (size_t i = 0; i < signature.size(); ++i)
        {
            columnIndexByComponent[signature[i]] = i;
        }
    }

    Archetype::~Archetype()
    {
        for (Chunk& chunk : chunks)
        {
            DestroyChunk(chunk);
        }
    }

    const std::vector<ComponentTypeId>& Archetype::Signature() const
    {
        return signature;
    }

    bool Archetype::Contains(const std::span<const ComponentTypeId> _requiredComponents) const
    {
        return std::includes(
            signature.begin(),
            signature.end(),
            _requiredComponents.begin(),
            _requiredComponents.end()
        );
    }

    bool Archetype::HasComponent(const ComponentTypeId _componentId) const
    {
        return columnIndexByComponent.contains(_componentId);
    }

    size_t Archetype::ChunkCount() const
    {
        return chunks.size();
    }

    size_t Archetype::ChunkEntityCount(const uint32_t _chunkIndex) const
    {
        CP_EXPECT_MSG(_chunkIndex < chunks.size(), "Invalid chunk index.");
        return chunks[_chunkIndex].count;
    }

    Entity Archetype::EntityAt(const uint32_t _chunkIndex, const uint32_t _rowIndex) const
    {
        CP_EXPECT_MSG(_chunkIndex < chunks.size(), "Invalid chunk index.");
        CP_EXPECT_MSG(_rowIndex < chunks[_chunkIndex].count, "Invalid row index.");
        return chunks[_chunkIndex].entities[_rowIndex];
    }

    void Archetype::SetEntityAt(const uint32_t _chunkIndex, const uint32_t _rowIndex, const Entity _entity)
    {
        CP_EXPECT_MSG(_chunkIndex < chunks.size(), "Invalid chunk index.");
        CP_EXPECT_MSG(_rowIndex < chunks[_chunkIndex].count, "Invalid row index.");
        chunks[_chunkIndex].entities[_rowIndex] = _entity;
    }

    void Archetype::AppendEntity(const Entity _entity, uint32_t& _outChunkIndex, uint32_t& _outRowIndex)
    {
        Chunk& chunk = EnsureChunkForAppend();
        const size_t row = chunk.count;
        chunk.entities[row] = _entity;
        ++chunk.count;

        _outChunkIndex = static_cast<uint32_t>(chunks.size() - 1);
        _outRowIndex = static_cast<uint32_t>(row);
    }

    void Archetype::DecrementChunkCount(const uint32_t _chunkIndex)
    {
        CP_EXPECT_MSG(_chunkIndex < chunks.size(), "Invalid chunk index.");
        CP_EXPECT_MSG(chunks[_chunkIndex].count > 0, "Chunk count is already zero.");
        --chunks[_chunkIndex].count;
    }

    void* Archetype::ComponentPtr(
        const ComponentTypeId _componentId,
        const uint32_t _chunkIndex,
        const uint32_t _rowIndex
    )
    {
        CP_EXPECT_MSG(_chunkIndex < chunks.size(), "Invalid chunk index.");
        const auto it = columnIndexByComponent.find(_componentId);
        if (it == columnIndexByComponent.end())
        {
            return nullptr;
        }

        const Chunk& chunk = chunks[_chunkIndex];
        CP_ASSERT_MSG(_rowIndex < chunk.count, "Invalid row index.");

        const size_t columnIndex = it->second;
        const ColumnLayout& layout = columnLayouts[columnIndex];
        return static_cast<std::byte*>(chunk.columns[columnIndex]) + static_cast<size_t>(_rowIndex) * layout.componentSize;
    }

    const void* Archetype::ComponentPtr(
        const ComponentTypeId _componentId,
        const uint32_t _chunkIndex,
        const uint32_t _rowIndex
    ) const
    {
        return const_cast<Archetype*>(this)->ComponentPtr(_componentId, _chunkIndex, _rowIndex);
    }

    void Archetype::DestroyComponent(
        const ComponentTypeId _componentId,
        const uint32_t _chunkIndex,
        const uint32_t _rowIndex
    )
    {
        const ColumnLayout& layout = LayoutFor(_componentId);
        void* ptr = ComponentPtr(_componentId, _chunkIndex, _rowIndex);
        layout.destroy(ptr);
    }

    void Archetype::MoveComponentTo(
        const ComponentTypeId _componentId,
        const uint32_t _sourceChunkIndex,
        const uint32_t _sourceRowIndex,
        Archetype& _target,
        const uint32_t _targetChunkIndex,
        const uint32_t _targetRowIndex
    )
    {
        const ColumnLayout& layout = LayoutFor(_componentId);
        void* src = ComponentPtr(_componentId, _sourceChunkIndex, _sourceRowIndex);
        void* dst = _target.ComponentPtr(_componentId, _targetChunkIndex, _targetRowIndex);
        CP_ASSERT_MSG(dst != nullptr, "Target archetype does not contain the requested component.");
        layout.moveConstruct(dst, src);
    }

    void Archetype::MoveRowWithinChunk(
        const uint32_t _chunkIndex,
        const uint32_t _sourceRowIndex,
        const uint32_t _targetRowIndex
    ) const
    {
        CP_EXPECT_MSG(_chunkIndex < chunks.size(), "Invalid chunk index.");
        const Chunk& chunk = chunks[_chunkIndex];
        CP_ASSERT_MSG(_sourceRowIndex < chunk.count && _targetRowIndex < chunk.count, "Invalid row index.");

        if (_sourceRowIndex == _targetRowIndex)
        {
            return;
        }

        for (size_t columnIndex = 0; columnIndex < columnLayouts.size(); ++columnIndex)
        {
            const ColumnLayout& layout = columnLayouts[columnIndex];

            void* src =
                static_cast<std::byte*>(chunk.columns[columnIndex])
                + static_cast<size_t>(_sourceRowIndex) * layout.componentSize;

            void* dst =
                static_cast<std::byte*>(chunk.columns[columnIndex])
                + static_cast<size_t>(_targetRowIndex) * layout.componentSize;

            layout.moveConstruct(dst, src);
            layout.destroy(src);
        }
    }

    const Archetype::ColumnLayout& Archetype::LayoutFor(const ComponentTypeId _componentId) const
    {
        const auto it = columnIndexByComponent.find(_componentId);
        CP_ASSERT_MSG(it != columnIndexByComponent.end(), "Component is not part of this archetype.");
        return columnLayouts[it->second];
    }

    Archetype::Chunk& Archetype::EnsureChunkForAppend()
    {
        if (chunks.empty() || chunks.back().count >= chunkCapacity)
        {
            chunks.push_back(CreateChunk());
        }

        return chunks.back();
    }

    Archetype::Chunk Archetype::CreateChunk() const
    {
        Chunk chunk{};
        chunk.count = 0;
        chunk.entities.resize(chunkCapacity);
        chunk.columns.reserve(columnLayouts.size());

        for (const ColumnLayout& layout : columnLayouts)
        {
            void* columnMemory = ::operator new[](
                layout.componentSize * chunkCapacity,
                std::align_val_t{ layout.componentAlignment }
            );
            
            chunk.columns.push_back(columnMemory);
        }

        return chunk;
    }

    void Archetype::DestroyChunk(Chunk& _chunk) const
    {
        for (size_t columnIndex = 0; columnIndex < columnLayouts.size(); ++columnIndex)
        {
            const ColumnLayout& layout = columnLayouts[columnIndex];
            for (size_t row = 0; row < _chunk.count; ++row)
            {
                void* ptr = static_cast<std::byte*>(_chunk.columns[columnIndex]) + row * layout.componentSize;
                layout.destroy(ptr);
            }

            ::operator delete[](_chunk.columns[columnIndex], std::align_val_t{ layout.componentAlignment });
        }

        _chunk.columns.clear();
        _chunk.entities.clear();
        _chunk.count = 0;
    }

    size_t SignatureHasher::operator()(const std::vector<ComponentTypeId>& _signature) const noexcept
    {
        size_t hash = 0;
        for (const ComponentTypeId componentId : _signature)
        {
            hash ^= std::hash<ComponentTypeId>{}(componentId) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }
}

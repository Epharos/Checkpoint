#include "World.hpp"

#include <cstring>

namespace cp::ecs
{
    namespace
    {
        class MemorySerializer final : public ISerializer
        {
        public:
            void Write(const std::span<const std::byte> _bytes) override
            {
                buffer.insert(buffer.end(), _bytes.begin(), _bytes.end());
            }

            std::vector<std::byte> buffer;
        };

        class MemoryDeserializer final : public IDeserializer
        {
        public:
            explicit MemoryDeserializer(const std::vector<std::byte>& _buffer)
                : buffer(_buffer)
            {
            }

            [[nodiscard]] bool Read(const std::span<std::byte> _bytes) override
            {
                if (cursor + _bytes.size() > buffer.size())
                {
                    return false;
                }

                std::memcpy(_bytes.data(), buffer.data() + cursor, _bytes.size());
                cursor += _bytes.size();
                return true;
            }

        private:
            const std::vector<std::byte>& buffer;
            size_t cursor = 0;
        };
    }

    World::World()
    {
        [[maybe_unused]] Archetype& init = GetOrCreateArchetype({});
    }

    World::~World()
    {
        Clear();
    }

    Entity World::CreateEntity()
    {
        uint32_t entityIndex = 0;

        if (!freeEntityIndices.empty())
        {
            entityIndex = freeEntityIndices.back();
            freeEntityIndices.pop_back();
        }
        else
        {
            entityIndex = static_cast<uint32_t>(entityRecords.size());
            entityRecords.emplace_back();
        }

        EntityRecord& record = entityRecords[entityIndex];
        if (record.generation == 0)
        {
            record.generation = 1;
        }

        record.alive = true;
        record.archetype = nullptr;
        record.chunkIndex = 0;
        record.rowIndex = 0;

        const Entity entity{entityIndex, record.generation};
        Archetype& emptyArchetype = GetOrCreateArchetype({});
        emptyArchetype.AppendEntity(entity, record.chunkIndex, record.rowIndex);
        record.archetype = &emptyArchetype;
        return entity;
    }

    bool World::DestroyEntity(const Entity _entity)
    {
        EntityRecord* record = TryGetRecord(_entity);
        if (!record)
        {
            return false;
        }

        RemoveEntityFromArchetype(*record);
        record->alive = false;
        record->archetype = nullptr;
        record->chunkIndex = 0;
        record->rowIndex = 0;

        ++record->generation;
        if (record->generation == 0)
        {
            record->generation = 1;
        }

        freeEntityIndices.push_back(_entity.index);
        return true;
    }

    bool World::IsAlive(const Entity _entity) const
    {
        return TryGetRecord(_entity) != nullptr;
    }

    void World::ReserveEntities(const size_t _count)
    {
        entityRecords.reserve(_count);
    }

    void World::Clear()
    {
        entityRecords.clear();
        freeEntityIndices.clear();
        archetypes.clear();
        archetypeBySignature.clear();
        requiredPlugins.clear();
        startupSystems.clear();

        [[maybe_unused]] Archetype& init = GetOrCreateArchetype({});
    }

    void World::ClearComponentTypes()
    {
        componentTypeByGuid.clear();
        componentTypeByTypeIndex.clear();
        componentTypes.clear();
    }

    void World::SetRequiredPlugins(std::vector<std::string> _pluginNames)
    {
        requiredPlugins = std::move(_pluginNames);
    }

    const std::vector<std::string>& World::GetRequiredPlugins() const
    {
        return requiredPlugins;
    }

    void World::SetStartupSystems(std::vector<TypeGuid> _systemGuids)
    {
        startupSystems = std::move(_systemGuids);
    }

    const std::vector<TypeGuid>& World::GetStartupSystems() const
    {
        return startupSystems;
    }

    ComponentTypeId World::RegisterComponentType(const ComponentTypeInfo& _info)
    {
        const auto componentId = static_cast<ComponentTypeId>(componentTypes.size());
        componentTypes.push_back(_info);
        return componentId;
    }

    World::EntityRecord* World::TryGetRecord(const Entity _entity)
    {
        if (!_entity.IsValid() || _entity.index >= entityRecords.size())
        {
            return nullptr;
        }

        EntityRecord& record = entityRecords[_entity.index];
        if (!record.alive || record.generation != _entity.generation)
        {
            return nullptr;
        }

        return &record;
    }

    const World::EntityRecord* World::TryGetRecord(const Entity _entity) const
    {
        return const_cast<World*>(this)->TryGetRecord(_entity);
    }

    Archetype& World::GetOrCreateArchetype(const std::vector<ComponentTypeId>& _signature)
    {
        if (const auto it = archetypeBySignature.find(_signature); it != archetypeBySignature.end())
        {
            return *it->second;
        }

        std::vector<Archetype::ColumnLayout> columnLayouts;
        columnLayouts.reserve(_signature.size());
        for (const ComponentTypeId componentId : _signature)
        {
            const ComponentTypeInfo& info = componentTypes[componentId];
            Archetype::ColumnLayout layout{};
            layout.componentId = componentId;
            layout.componentSize = info.size;
            layout.componentAlignment = info.alignment;
            layout.moveConstruct = info.moveConstruct;
            layout.destroy = info.destroy;
            columnLayouts.push_back(layout);
        }

        auto archetype = std::make_unique<Archetype>(_signature, std::move(columnLayouts), ResolveChunkCapacity(_signature));
        Archetype* archetypeRaw = archetype.get();

        archetypes.push_back(std::move(archetype));
        archetypeBySignature[_signature] = archetypeRaw;
        return *archetypeRaw;
    }

    void* World::GetComponentPtr(const EntityRecord& _record, const ComponentTypeId _componentId)
    {
        CP_ASSERT_MSG(_record.archetype != nullptr, "Entity record is not attached to an archetype.");
        return _record.archetype->ComponentPtr(_componentId, _record.chunkIndex, _record.rowIndex);
    }

    const void* World::GetComponentPtr(const EntityRecord& _record, const ComponentTypeId _componentId) const
    {
        return const_cast<World*>(this)->GetComponentPtr(_record, _componentId);
    }

    void World::RemoveEntityFromArchetype(const EntityRecord& _record)
    {
        CP_ASSERT_MSG(_record.archetype != nullptr, "Entity record is not attached to an archetype.");

        Archetype& archetype = *_record.archetype;
        const uint32_t chunkIndex = _record.chunkIndex;
        const uint32_t rowIndex = _record.rowIndex;
        const auto lastRow = static_cast<uint32_t>(archetype.ChunkEntityCount(chunkIndex) - 1);

        for (const ComponentTypeId componentId : archetype.Signature())
        {
            archetype.DestroyComponent(componentId, chunkIndex, rowIndex);
        }

        if (rowIndex != lastRow)
        {
            const Entity movedEntity = archetype.EntityAt(chunkIndex, lastRow);
            archetype.SetEntityAt(chunkIndex, rowIndex, movedEntity);
            archetype.MoveRowWithinChunk(chunkIndex, lastRow, rowIndex);

            EntityRecord& movedRecord = entityRecords[movedEntity.index];
            movedRecord.chunkIndex = chunkIndex;
            movedRecord.rowIndex = rowIndex;
        }

        archetype.DecrementChunkCount(chunkIndex);
    }

    void World::MoveEntityToArchetype(
        const Entity _entity,
        EntityRecord& _record,
        const std::vector<ComponentTypeId>& _targetSignature,
        const ComponentTypeInfo* _addedType,
        const void* _addedValue,
        const ComponentTypeId _removedComponentId
    )
    {
        CP_ASSERT_MSG(_record.archetype != nullptr, "Entity record is not attached to an archetype.");

        Archetype& sourceArchetype = *_record.archetype;
        const uint32_t sourceChunkIndex = _record.chunkIndex;
        const uint32_t sourceRowIndex = _record.rowIndex;

        Archetype& targetArchetype = GetOrCreateArchetype(_targetSignature);
        uint32_t targetChunkIndex = 0;
        uint32_t targetRowIndex = 0;
        targetArchetype.AppendEntity(_entity, targetChunkIndex, targetRowIndex);

        for (const ComponentTypeId componentId : sourceArchetype.Signature())
        {
            if (_removedComponentId != InvalidComponentType && componentId == _removedComponentId)
            {
                continue;
            }

            if (!targetArchetype.HasComponent(componentId))
            {
                continue;
            }

            sourceArchetype.MoveComponentTo(componentId, sourceChunkIndex, sourceRowIndex, targetArchetype, targetChunkIndex, targetRowIndex);
        }

        if (_addedType&& _addedValue)
        {
            const auto it = componentTypeByTypeIndex.find(_addedType->type);
            CP_ASSERT_MSG(it != componentTypeByTypeIndex.end(), "Added component type is unknown.");

            const ComponentTypeId componentId = it->second;
            void* dst = targetArchetype.ComponentPtr(componentId, targetChunkIndex, targetRowIndex);
            _addedType->copyConstruct(dst, _addedValue);
        }

        for (const ComponentTypeId componentId : sourceArchetype.Signature())
        {
            sourceArchetype.DestroyComponent(componentId, sourceChunkIndex, sourceRowIndex);
        }

        const uint32_t sourceLastRow = static_cast<uint32_t>(sourceArchetype.ChunkEntityCount(sourceChunkIndex) - 1);
        if (sourceRowIndex != sourceLastRow)
        {
            const Entity movedEntity = sourceArchetype.EntityAt(sourceChunkIndex, sourceLastRow);
            sourceArchetype.SetEntityAt(sourceChunkIndex, sourceRowIndex, movedEntity);
            sourceArchetype.MoveRowWithinChunk(sourceChunkIndex, sourceLastRow, sourceRowIndex);

            EntityRecord& movedRecord = entityRecords[movedEntity.index];
            movedRecord.chunkIndex = sourceChunkIndex;
            movedRecord.rowIndex = sourceRowIndex;
        }

        sourceArchetype.DecrementChunkCount(sourceChunkIndex);

        _record.archetype = &targetArchetype;
        _record.chunkIndex = targetChunkIndex;
        _record.rowIndex = targetRowIndex;
    }

    bool World::AddComponentFromUntyped(const Entity _entity, const ComponentTypeId _componentId, const void* _valuePtr)
    {
        EntityRecord* record = TryGetRecord(_entity);
        if (!record)
        {
            return false;
        }

        if (GetComponentPtr(*record, _componentId))
        {
            return false;
        }

        std::vector<ComponentTypeId> targetSignature = record->archetype->Signature();
        targetSignature.insert(std::lower_bound(targetSignature.begin(), targetSignature.end(), _componentId), _componentId);

        const ComponentTypeInfo& addedType = componentTypes[_componentId];
        MoveEntityToArchetype(_entity, *record, targetSignature, &addedType, _valuePtr, InvalidComponentType);
        return true;
    }

    bool World::DeserializeComponentPayload(const Entity _entity, const ComponentTypeId _componentId, const std::vector<std::byte>& _payload)
    {
        EntityRecord* record = TryGetRecord(_entity);
        if (!record)
        {
            return false;
        }

        const ComponentTypeInfo& info = componentTypes[_componentId];
        if (!info.defaultConstruct || !info.deserializeBinary)
        {
            return false;
        }

        std::vector<std::byte> scratch(info.size + info.alignment);
        void* raw = scratch.data();
        size_t space = scratch.size();
        void* aligned = std::align(info.alignment, info.size, raw, space);
        if (!aligned)
        {
            return false;
        }

        info.defaultConstruct(aligned);

        MemoryDeserializer reader(_payload);
        const bool success = info.deserializeBinary(aligned, reader);
        if (success)
        {
            const bool added = AddComponentFromUntyped(_entity, _componentId, aligned);
            info.destroy(aligned);
            return added;
        }

        info.destroy(aligned);
        return false;
    }

    size_t World::ResolveChunkCapacity(const std::vector<ComponentTypeId>& _signature) const
    {
        if (_signature.empty())
        {
            return 1024;
        }

        size_t bytesPerEntity = 0;
        for (const ComponentTypeId componentId : _signature)
        {
            bytesPerEntity += std::max<size_t>(componentTypes[componentId].size, 1);
        }

        constexpr size_t targetChunkBytes = 16 * 1024;
        const size_t rawCapacity = targetChunkBytes / std::max<size_t>(bytesPerEntity, 1);
        return std::clamp(rawCapacity, size_t { 16 }, size_t { 1024 });
    }

    bool World::SerializeBinary(ISerializer& _serializer) const
    {
        _serializer.WritePod(SerializationMagic);
        _serializer.WritePod(SerializationVersion);

        _serializer.WritePod(static_cast<uint32_t>(requiredPlugins.size()));
        for (const std::string& pluginName : requiredPlugins)
        {
            _serializer.WriteString(pluginName);
        }

        _serializer.WritePod(static_cast<uint32_t>(startupSystems.size()));
        for (const TypeGuid systemGuid : startupSystems)
        {
            _serializer.WritePod(systemGuid);
        }

        std::vector<Entity> aliveEntities;
        aliveEntities.reserve(entityRecords.size());
        for (size_t i = 0; i < entityRecords.size(); ++i)
        {
            const Entity candidate{static_cast<uint32_t>(i), entityRecords[i].generation};
            if (IsAlive(candidate))
            {
                aliveEntities.push_back(candidate);
            }
        }

        _serializer.WritePod(static_cast<uint32_t>(aliveEntities.size()));
        for (const Entity entity : aliveEntities)
        {
            _serializer.WritePod(entity.index);
            _serializer.WritePod(entity.generation);

            const EntityRecord* record = TryGetRecord(entity);
            CP_ASSERT(record != nullptr);

            uint32_t serializableCount = 0;
            for (const ComponentTypeId componentId : record->archetype->Signature())
            {
                const ComponentTypeInfo& info = componentTypes[componentId];
                if (info.typeGuid != 0 && info.serializeBinary && info.deserializeBinary)
                {
                    ++serializableCount;
                }
            }

            _serializer.WritePod(serializableCount);

            for (const ComponentTypeId componentId : record->archetype->Signature())
            {
                const ComponentTypeInfo& info = componentTypes[componentId];
                if (info.typeGuid == 0 || !info.serializeBinary || !info.deserializeBinary)
                {
                    continue;
                }

                _serializer.WritePod(info.typeGuid);
                _serializer.WritePod(info.schemaVersion);

                MemorySerializer payloadWriter{};
                const void* componentPtr = GetComponentPtr(*record, componentId);
                info.serializeBinary(componentPtr, payloadWriter);

                _serializer.WritePod(static_cast<uint32_t>(payloadWriter.buffer.size()));
                if (!payloadWriter.buffer.empty())
                {
                    _serializer.Write(std::span<const std::byte>(payloadWriter.buffer.data(), payloadWriter.buffer.size()));
                }
            }
        }

        return true;
    }

    bool World::DeserializeBinary(IDeserializer& _deserializer)
    {
        uint32_t magic = 0;
        uint32_t version = 0;
        if (!_deserializer.ReadPod(magic) || !_deserializer.ReadPod(version))
        {
            return false;
        }

        if (magic != SerializationMagic || version != SerializationVersion)
        {
            return false;
        }

        Clear();

        uint32_t pluginCount = 0;
        if (!_deserializer.ReadPod(pluginCount))
        {
            return false;
        }

        requiredPlugins.clear();
        requiredPlugins.reserve(pluginCount);
        for (uint32_t i = 0; i < pluginCount; ++i)
        {
            std::string pluginName;
            if (!_deserializer.ReadString(pluginName))
            {
                return false;
            }
            requiredPlugins.push_back(std::move(pluginName));
        }

        uint32_t systemCount = 0;
        if (!_deserializer.ReadPod(systemCount))
        {
            return false;
        }

        startupSystems.clear();
        startupSystems.reserve(systemCount);
        for (uint32_t i = 0; i < systemCount; ++i)
        {
            TypeGuid guid = 0;
            if (!_deserializer.ReadPod(guid))
            {
                return false;
            }
            startupSystems.push_back(guid);
        }

        uint32_t entityCount = 0;
        if (!_deserializer.ReadPod(entityCount))
        {
            return false;
        }

        for (uint32_t i = 0; i < entityCount; ++i)
        {
            uint32_t serializedIndex = 0;
            uint32_t serializedGeneration = 0;
            if (!_deserializer.ReadPod(serializedIndex) || !_deserializer.ReadPod(serializedGeneration))
            {
                return false;
            }

            const Entity createdEntity = CreateEntity();
            EntityRecord* record = TryGetRecord(createdEntity);
            CP_ASSERT(record != nullptr);
            record->generation = serializedGeneration;
            const Entity entity{createdEntity.index, serializedGeneration};

            uint32_t componentCount = 0;
            if (!_deserializer.ReadPod(componentCount))
            {
                return false;
            }

            for (uint32_t c = 0; c < componentCount; ++c)
            {
                ComponentTypeGuid componentGuid = 0;
                uint32_t schemaVersion = 0;
                if (!_deserializer.ReadPod(componentGuid) || !_deserializer.ReadPod(schemaVersion))
                {
                    return false;
                }

                uint32_t payloadSize = 0;
                if (!_deserializer.ReadPod(payloadSize))
                {
                    return false;
                }

                std::vector<std::byte> payload(payloadSize);
                if (payloadSize > 0 && !_deserializer.Read(std::span<std::byte>(payload.data(), payload.size())))
                {
                    return false;
                }

                const auto typeIt = componentTypeByGuid.find(componentGuid);
                if (typeIt == componentTypeByGuid.end())
                {
                    return false;
                }

                const ComponentTypeInfo& info = componentTypes[typeIt->second];
                if (schemaVersion != info.schemaVersion)
                {
                    return false;
                }

                if (!DeserializeComponentPayload(entity, typeIt->second, payload))
                {
                    return false;
                }
            }
        }

        return true;
    }
}

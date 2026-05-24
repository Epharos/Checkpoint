#include "Spawner.hpp"

#include <Common/IO/FileHelper.hpp>

#include "ComponentRegistrationContext.hpp"

namespace cp
{
    std::string_view SpawnerRegistrar::Name() const
    {
        return "Spawner";
    }

    ecs::TypeGuid SpawnerRegistrar::ComponentGuid() const
    {
        return SpawnerGuid;
    }

    void SpawnerRegistrar::Register(ecs::World &_world) const
    {
        _world.RegisterComponentSerialization<Spawner>(
            SpawnerGuid,
            1,
            std::string(Name()),
            [](const Spawner& _value, ISerializer& _serializer)
            {
                _serializer.WritePod(_value.delay);
                _serializer.WriteString(GetRelativePath((_value.mesh.GetAssetID())).string());
            },
            [](Spawner& _value, IDeserializer& _deserializer)
            {
                std::string meshAssetPath;

                if (!_deserializer.ReadPod(_value.delay) ||
                    !_deserializer.ReadString(meshAssetPath))
                {
                    return false;
                }

                if (meshAssetPath.empty())
                {
                    _value.mesh = AssetHandle<Mesh>{};
                    return true;
                }

                auto& meshManager = cp::GetComponentRegistrationContext().GetAssetRegistry().Get<Mesh>();
                _value.mesh = meshManager.Load(ResolveAssetPath(meshAssetPath));
                return _value.mesh.IsValid();
            }
        );
    }

    std::string_view SpawnerAuthoring::Name() const
    {
        return "Spawner";
    }

    uint64_t SpawnerAuthoring::ComponentGuid() const
    {
        return SpawnerGuid;
    }

    bool SpawnerAuthoring::HasComponent(const cp::ecs::World &_world, const cp::ecs::Entity &_entity) const
    {
        return _world.HasComponent<Spawner>(_entity);
    }

    bool SpawnerAuthoring::AddComponent(cp::ecs::World &_world, const cp::ecs::Entity &_entity) const
    {
        if (HasComponent(_world, _entity))
        {
            return false;
        }

        _world.AddComponent<Spawner>(_entity, Spawner{});
        return true;
    }

    bool SpawnerAuthoring::RemoveComponent(cp::ecs::World &_world, const cp::ecs::Entity &_entity) const
    {
        if (!HasComponent(_world, _entity))
        {
            return false;
        }

        return _world.RemoveComponent<Spawner>(_entity);
    }

    std::vector<cp::AuthoringSectionDescriptor> SpawnerAuthoring::BuildSections(
        const cp::ecs::World &_world,
        const cp::ecs::Entity &_entity
    ) const
    {
        if (!HasComponent(_world, _entity))
        {
            return {};
        }

        const auto& spawner = _world.GetComponent<Spawner>(_entity);

        return {
            cp::AuthoringSectionDescriptor {
                .id = "spawner",
                .title = "Spawner",
                .fields = {
                    cp::AuthoringFieldDescriptor {
                        .id = "delay",
                        .label = "Delay",
                        .valueType = cp::AuthoringValueType::Float,
                        .value = spawner.delay
                    },
                    cp::AuthoringFieldDescriptor{
                        .id = "meshPath",
                        .label = "Mesh Asset",
                        .valueType = cp::AuthoringValueType::String,
                        .inputType = cp::AuthoringInputType::FilePath,
                        .value = spawner.mesh.IsValid()
                            ? GetRelativePath(spawner.mesh.GetAssetID()).string()
                            : std::string{},
                        .readOnly = false,
                        .fileExtensions = { "fbx", "obj", "gltf" }
                    }
                }
            }
        };
    }

    bool SpawnerAuthoring::ApplyValue(
        cp::ecs::World &_world,
        const cp::ecs::Entity &_entity,
        const std::string_view _fieldId,
        const cp::AuthoringValue &_value,
        std::string &_outError
    ) const
    {
        if (!HasComponent(_world, _entity))
        {
            _outError = "Spawner component is missing on selected entity.";
            return false;
        }

        if (_fieldId == "meshPath")
        {
            const std::string* stringValue = std::get_if<std::string>(&_value);
            if (stringValue == nullptr)
            {
                _outError = "Mesh asset path expects a string value.";
                return false;
            }

            auto& spawner = _world.GetComponent<Spawner>(_entity);

            if (stringValue->empty())
            {
                spawner.mesh = AssetHandle<Mesh>{};
                return true;
            }

            auto& meshManager = cp::GetComponentRegistrationContext().GetAssetRegistry().Get<Mesh>();
            const auto meshPath = ResolveAssetPath(*stringValue);
            if (!std::filesystem::exists(meshPath))
            {
                _outError = "Mesh file not found: " + *stringValue;
                return false;
            }

            spawner.mesh = meshManager.Load(meshPath);
            if (!spawner.mesh.IsValid())
            {
                _outError = "Failed to load mesh: " + *stringValue;
                return false;
            }

            return true;
        }

        const double* doubleValue = std::get_if<double>(&_value);
        if (doubleValue == nullptr)
        {
            _outError = "Float value is nullptr";
            return false;
        }

        auto& spawner = _world.GetComponent<Spawner>(_entity);
        spawner.delay = *doubleValue;
        return true;
    }
}

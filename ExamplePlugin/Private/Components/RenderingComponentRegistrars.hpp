#pragma once

#include <ECS/ECS.hpp>
#include <Resources/AssetRegistry.hpp>
#include <Resources/Mesh.hpp>

#include "RenderingComponents.hpp"

#include <stdexcept>

namespace cp
{
    inline AssetRegistry* g_pluginAssetRegistry = nullptr;

    inline void SetPluginAssetRegistry(AssetRegistry* _assetRegistry)
    {
        g_pluginAssetRegistry = _assetRegistry;
    }

    inline AssetRegistry& GetPluginAssetRegistry()
    {
        if (g_pluginAssetRegistry == nullptr)
        {
            throw std::runtime_error("Plugin asset registry not configured.");
        }

        return *g_pluginAssetRegistry;
    }

    inline constexpr cp::ecs::TypeGuid Transform3DGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.Transform3D");
    inline constexpr cp::ecs::TypeGuid CameraGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.Camera");
    inline constexpr cp::ecs::TypeGuid MeshRendererGuid = cp::ecs::MakeTypeGuid("ExamplePlugin.MeshRenderer");

    class Transform3DRegistrar final : public cp::ecs::IComponentRegistrar
    {
    public:
        [[nodiscard]] std::string_view Name() const override
        {
            return "Transform3D";
        }

        [[nodiscard]] cp::ecs::TypeGuid ComponentGuid() const override
        {
            return Transform3DGuid;
        }

        void Register(cp::ecs::World& _world) const override
        {
            _world.RegisterComponentSerialization<Transform3D>(
                Transform3DGuid,
                1,
                std::string(Name()),
                [](const Transform3D& _value, cp::ISerializer& _serializer)
                {
                    _serializer.WritePod(_value.x);
                    _serializer.WritePod(_value.y);
                    _serializer.WritePod(_value.z);
                    _serializer.WritePod(_value.pitch);
                    _serializer.WritePod(_value.yaw);
                    _serializer.WritePod(_value.roll);
                    _serializer.WritePod(_value.scaleX);
                    _serializer.WritePod(_value.scaleY);
                    _serializer.WritePod(_value.scaleZ);
                },
                [](Transform3D& _value, cp::IDeserializer& _deserializer)
                {
                    return _deserializer.ReadPod(_value.x)
                        && _deserializer.ReadPod(_value.y)
                        && _deserializer.ReadPod(_value.z)
                        && _deserializer.ReadPod(_value.pitch)
                        && _deserializer.ReadPod(_value.yaw)
                        && _deserializer.ReadPod(_value.roll)
                        && _deserializer.ReadPod(_value.scaleX)
                        && _deserializer.ReadPod(_value.scaleY)
                        && _deserializer.ReadPod(_value.scaleZ);
                }
            );
        }
    };

    class CameraRegistrar final : public cp::ecs::IComponentRegistrar
    {
    public:
        [[nodiscard]] std::string_view Name() const override
        {
            return "Camera";
        }

        [[nodiscard]] cp::ecs::TypeGuid ComponentGuid() const override
        {
            return CameraGuid;
        }

        void Register(cp::ecs::World& _world) const override
        {
            _world.RegisterComponentSerialization<Camera>(
                CameraGuid,
                1,
                std::string(Name()),
                [](const Camera& _value, cp::ISerializer& _serializer)
                {
                    _serializer.WritePod(_value.fovYDegrees);
                    _serializer.WritePod(_value.nearPlane);
                    _serializer.WritePod(_value.farPlane);
                    _serializer.WritePod(_value.isPrimary);
                    _serializer.WritePod(_value.enabled);
                },
                [](Camera& _value, cp::IDeserializer& _deserializer)
                {
                    return _deserializer.ReadPod(_value.fovYDegrees)
                        && _deserializer.ReadPod(_value.nearPlane)
                        && _deserializer.ReadPod(_value.farPlane)
                        && _deserializer.ReadPod(_value.isPrimary)
                        && _deserializer.ReadPod(_value.enabled);
                }
            );
        }
    };

    class MeshRendererRegistrar final : public cp::ecs::IComponentRegistrar
    {
    public:
        [[nodiscard]] std::string_view Name() const override
        {
            return "MeshRenderer";
        }

        [[nodiscard]] cp::ecs::TypeGuid ComponentGuid() const override
        {
            return MeshRendererGuid;
        }

        void Register(cp::ecs::World& _world) const override
        {
            _world.RegisterComponentSerialization<MeshRenderer>(
                MeshRendererGuid,
                1,
                std::string(Name()),
                [](const MeshRenderer& _value, cp::ISerializer& _serializer)
                {
                    _serializer.WriteString(GetRelativePath(_value.meshId.GetAssetID()).string());
                    _serializer.WritePod(_value.visible);
                },
                [](MeshRenderer& _value, cp::IDeserializer& _deserializer)
                {
                    std::string meshAssetPath;
                    if (!_deserializer.ReadString(meshAssetPath)
                        || !_deserializer.ReadPod(_value.visible))
                    {
                        return false;
                    }

                    if (meshAssetPath.empty())
                    {
                        _value.meshId = AssetHandle<Mesh>{};
                        return true;
                    }

                    auto& meshManager = GetPluginAssetRegistry().Get<Mesh>();
                    _value.meshId = meshManager.Load(FindFileInParentTree(meshAssetPath));
                    return _value.meshId.IsValid();
                }
            );
        }
    };
}

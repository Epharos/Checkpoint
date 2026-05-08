#include <Runtime/Scene/Scene.hpp>

#include <Common/Serialization/ISerializer.hpp>

#include <ECS/World.hpp>

namespace cp::runtime
{
    Scene::Scene() : world(std::make_unique<cp::ecs::World>()) {}

    Scene::~Scene() = default;
    Scene::Scene(Scene&&) noexcept = default;
    Scene& Scene::operator=(Scene&&) noexcept = default;

    cp::ecs::World& Scene::GetWorld()
    {
        return *world;
    }

    const cp::ecs::World& Scene::GetWorld() const
    {
        return *world;
    }

    const std::vector<std::string>& Scene::GetActivePassNames() const
    {
        return description.activePassNames;
    }

    void Scene::SetActivePassNames(std::vector<std::string> _passNames)
    {
        description.activePassNames = std::move(_passNames);
    }

    const std::vector<std::string>& Scene::GetEnabledSystemGuids() const
    {
        return description.systemsConfig.enabledSystemGuids;
    }

    void Scene::SetEnabledSystemGuids(std::vector<std::string> _guids)
    {
        description.systemsConfig.enabledSystemGuids = std::move(_guids);
    }

    void Scene::SetFrameGraphReference(const cp::FrameGraph* _frameGraph)
    {
        frameGraphRef = _frameGraph;
    }

    const cp::FrameGraph* Scene::GetFrameGraphReference() const
    {
        return frameGraphRef;
    }

    const cp::scene::SceneDescription& Scene::GetDescription() const
    {
        return description;
    }

    const std::string& Scene::GetName() const
    {
        return description.name;
    }

    void Scene::SetName(std::string _name)
    {
        description.name = std::move(_name);
    }

    void Scene::Initialize(const std::string& _name)
    {
        world = std::make_unique<cp::ecs::World>();
        frameGraphRef = nullptr;
        description = {};
        description.name = _name;
    }

    void Scene::Clear()
    {
        world->Clear();
        description.activePassNames.clear();
        description.systemsConfig.enabledSystemGuids.clear();
        frameGraphRef = nullptr;
    }

    bool Scene::SerializeTo(cp::ISerializer& _serializer) const
    {
        _serializer.WritePod(cp::scene::SceneDescription::MAGIC);
        _serializer.WritePod(cp::scene::SceneDescription::VERSION);
        _serializer.WriteString(description.name);
        _serializer.WritePod(description.sceneGuid.high);
        _serializer.WritePod(description.sceneGuid.low);
        _serializer.WritePod(description.lastModified);

        const auto systemCount = static_cast<uint32_t>(description.systemsConfig.enabledSystemGuids.size());
        _serializer.WritePod(systemCount);
        for (const auto& guid : description.systemsConfig.enabledSystemGuids)
            _serializer.WriteString(guid);

        const auto passCount = static_cast<uint32_t>(description.activePassNames.size());
        _serializer.WritePod(passCount);
        for (const auto& name : description.activePassNames)
            _serializer.WriteString(name);

        return world->SerializeBinary(_serializer);
    }

    bool Scene::DeserializeFrom(cp::IDeserializer& _deserializer)
    {
        uint32_t magic = 0;
        uint32_t version = 0;
        if (!_deserializer.ReadPod(magic) || magic != cp::scene::SceneDescription::MAGIC)
            return false;
        if (!_deserializer.ReadPod(version))
            return false;

        if (!_deserializer.ReadString(description.name)) return false;
        if (!_deserializer.ReadPod(description.sceneGuid.high)) return false;
        if (!_deserializer.ReadPod(description.sceneGuid.low)) return false;
        if (!_deserializer.ReadPod(description.lastModified)) return false;

        uint32_t systemCount = 0;
        if (!_deserializer.ReadPod(systemCount)) return false;
        description.systemsConfig.enabledSystemGuids.resize(systemCount);
        for (auto& guid : description.systemsConfig.enabledSystemGuids)
            if (!_deserializer.ReadString(guid)) return false;

        uint32_t passCount = 0;
        if (!_deserializer.ReadPod(passCount)) return false;
        description.activePassNames.resize(passCount);
        for (auto& name : description.activePassNames)
            if (!_deserializer.ReadString(name)) return false;

        // Reuse the existing World so any external pointer stays valid
        world->Clear();
        return world->DeserializeBinary(_deserializer);
    }
}

#include <Runtime/Scene/Scene.hpp>

#include <Common/Core/Registry.hpp>
#include <Common/Serialization/ISerializer.hpp>

#include <ECS/System.hpp>
#include <ECS/World.hpp>

#include <cstring>

#include "Common/Authoring.hpp"

namespace
{
    class BlobSerializer final : public cp::ISerializer
    {
    public:
        void Write(std::span<const std::byte> _bytes) override
        {
            bytes.insert(bytes.end(), _bytes.begin(), _bytes.end());
        }

        std::vector<std::byte> bytes;
    };

    class BlobDeserializer final : public cp::IDeserializer
    {
    public:
        explicit BlobDeserializer(const std::vector<std::byte>& _bytes)
            : data(_bytes), cursor(0) {}

        bool Read(std::span<std::byte> _dst) override
        {
            if (cursor + _dst.size() > data.size()) return false;
            std::memcpy(_dst.data(), data.data() + cursor, _dst.size());
            cursor += _dst.size();
            return true;
        }

    private:
        const std::vector<std::byte>& data;
        size_t cursor = 0;
    };

    void WriteValue(cp::ISerializer& s, bool v) { s.WritePod(v); }
    void WriteValue(cp::ISerializer& s, int64_t v) { s.WritePod(v); }
    void WriteValue(cp::ISerializer& s, double v) { s.WritePod(v); }
    void WriteValue(cp::ISerializer& s, const std::string& v) { s.WriteString(v); }
    void WriteValue(cp::ISerializer& s, const cp::AuthoringVec3& v)
    {
        s.WritePod(v.x);
        s.WritePod(v.y);
        s.WritePod(v.z);
    }

    void WriteAuthoringValue(cp::ISerializer& s, const cp::AuthoringValue& val)
    {
        s.WritePod(static_cast<uint8_t>(val.index()));
        std::visit([&s](const auto& v) { WriteValue(s, v); }, val);
    }

    bool ReadValue(cp::IDeserializer& d, bool& v) { return d.ReadPod(v); }
    bool ReadValue(cp::IDeserializer& d, int64_t& v) { return d.ReadPod(v); }
    bool ReadValue(cp::IDeserializer& d, double& v) { return d.ReadPod(v); }
    bool ReadValue(cp::IDeserializer& d, std::string& v) { return d.ReadString(v); }
    bool ReadValue(cp::IDeserializer& d, cp::AuthoringVec3& v)
    {
        return d.ReadPod(v.x) && d.ReadPod(v.y) && d.ReadPod(v.z);
    }

    template<typename Variant, std::size_t I = 0>
    bool ReadVariantByIndex(cp::IDeserializer& d, Variant& val, uint8_t index)
    {
        if constexpr (I < std::variant_size_v<Variant>)
        {
            if (index == I)
            {
                std::variant_alternative_t<I, Variant> v{};
                if (!ReadValue(d, v)) return false;
                val = std::move(v);
                return true;
            }

            return ReadVariantByIndex<Variant, I + 1>(d, val, index);
        }

        return false;
    }

    bool ReadAuthoringValue(cp::IDeserializer& d, cp::AuthoringValue& val)
    {
        uint8_t tag = 0;
        if (!d.ReadPod(tag)) return false;
        return ReadVariantByIndex<cp::AuthoringValue>(d, val, tag);
    }

    void WriteAuthoringParams(cp::ISerializer& s, const cp::AuthoringParams& params)
    {
        s.WritePod(static_cast<uint32_t>(params.size()));

        for (const auto& [fieldId, value] : params)
        {
            s.WriteString(fieldId);
            WriteAuthoringValue(s, value);
        }
    }

    bool ReadAuthoringParams(cp::IDeserializer& d, cp::AuthoringParams& params)
    {
        uint32_t count = 0;
        if (!d.ReadPod(count)) return false;
        params.clear();

        for (uint32_t i = 0; i < count; ++i)
        {
            std::string fieldId;
            cp::AuthoringValue value;
            if (!d.ReadString(fieldId)) return false;
            if (!ReadAuthoringValue(d, value)) return false;
            params.emplace(std::move(fieldId), std::move(value));
        }

        return true;
    }

    void WriteParamsMap(cp::ISerializer& s,
        const std::unordered_map<std::string, cp::AuthoringParams>& map)
    {
        s.WritePod(static_cast<uint32_t>(map.size()));

        for (const auto& [key, params] : map)
        {
            s.WriteString(key);
            WriteAuthoringParams(s, params);
        }
    }

    bool ReadParamsMap(cp::IDeserializer& d,
        std::unordered_map<std::string, cp::AuthoringParams>& map)
    {
        uint32_t count = 0;
        if (!d.ReadPod(count)) return false;
        map.clear();
        for (uint32_t i = 0; i < count; ++i)
        {
            std::string key;
            cp::AuthoringParams params;
            if (!d.ReadString(key)) return false;
            if (!ReadAuthoringParams(d, params)) return false;
            map.emplace(std::move(key), std::move(params));
        }
        return true;
    }
}

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

    cp::scene::SceneDescription& Scene::GetDescription()
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
        description.passBlobs.clear();
        frameGraphRef = nullptr;
        activeSystems.clear();
        pendingSystemBlobs.clear();
    }

    const std::unordered_map<std::string, std::vector<std::byte>>& Scene::GetPassBlobs() const
    {
        return description.passBlobs;
    }

    void Scene::SetPassBlob(std::string _typeName, std::vector<std::byte> _blob)
    {
        description.passBlobs[std::move(_typeName)] = std::move(_blob);
    }

    void Scene::ClearPassBlobs()
    {
        description.passBlobs.clear();
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

        for (size_t i = 0; i < description.systemsConfig.enabledSystemGuids.size(); ++i)
        {
            if (i < activeSystems.size() && activeSystems[i])
            {
                BlobSerializer blobSerializer;
                activeSystems[i]->Serialize(blobSerializer);
                const auto blobSize = static_cast<uint32_t>(blobSerializer.bytes.size());
                _serializer.WritePod(blobSize);
                if (blobSize > 0)
                    _serializer.Write(std::span<const std::byte>(blobSerializer.bytes.data(), blobSize));
            }
            else
            {
                // System not active: write empty blob
                _serializer.WritePod(static_cast<uint32_t>(0));
            }
        }

        _serializer.WritePod(static_cast<uint32_t>(description.passBlobs.size()));

        for (const auto& [typeName, blob] : description.passBlobs)
        {
            _serializer.WriteString(typeName);
            _serializer.WritePod(static_cast<uint32_t>(blob.size()));
            if (!blob.empty())
                _serializer.Write(std::span<const std::byte>(blob.data(), blob.size()));
        }

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

        pendingSystemBlobs.clear();
        for (const auto& guid : description.systemsConfig.enabledSystemGuids)
        {
            uint32_t blobSize = 0;
            if (!_deserializer.ReadPod(blobSize)) return false;
            std::vector<std::byte> blob(blobSize);
            if (blobSize > 0 && !_deserializer.Read(std::span<std::byte>(blob.data(), blobSize)))
                return false;
            if (blobSize > 0)
                pendingSystemBlobs[guid] = std::move(blob);
        }

        description.passBlobs.clear();
        uint32_t passBlobCount = 0;
        if (!_deserializer.ReadPod(passBlobCount)) return false;
        for (uint32_t i = 0; i < passBlobCount; ++i)
        {
            std::string typeName;
            uint32_t blobSize = 0;
            if (!_deserializer.ReadString(typeName)) return false;
            if (!_deserializer.ReadPod(blobSize)) return false;
            std::vector<std::byte> blob(blobSize);
            if (blobSize > 0 && !_deserializer.Read(std::span<std::byte>(blob.data(), blobSize)))
                return false;
            if (blobSize > 0)
                description.passBlobs[std::move(typeName)] = std::move(blob);
        }

        world->Clear();
        return world->DeserializeBinary(_deserializer);
    }

    size_t Scene::InitializeSystems(const cp::Registry<cp::ecs::ISystem>& _registry)
    {
        for (auto& sys : activeSystems)
        {
            if (sys)
            {
                BlobSerializer blobSerializer;
                sys->Serialize(blobSerializer);
                if (!blobSerializer.bytes.empty())
                    pendingSystemBlobs[cp::ecs::GuidToRegistryKey(sys->SystemGuid())] = std::move(blobSerializer.bytes);
            }
        }

        activeSystems.clear();
        size_t count = 0;
        for (const std::string& guid : description.systemsConfig.enabledSystemGuids)
        {
            if (!_registry.Contains(guid))
            {
                activeSystems.push_back(nullptr); // keep index alignment
                continue;
            }

            auto system = _registry.Create(guid);
            if (!system)
            {
                activeSystems.push_back(nullptr);
                continue;
            }

            const auto blobIt = pendingSystemBlobs.find(guid);
            if (blobIt != pendingSystemBlobs.end() && !blobIt->second.empty())
            {
                BlobDeserializer blobDeserializer(blobIt->second);
                system->Deserialize(blobDeserializer);
            }

            activeSystems.push_back(std::move(system));
            ++count;
        }

        pendingSystemBlobs.clear();
        return count;
    }

    void Scene::ShutdownSystems()
    {
        activeSystems.clear();
    }

    cp::ecs::ISystem* Scene::FindActiveSystem(std::string_view _guid)
    {
        const auto& guids = description.systemsConfig.enabledSystemGuids;
        for (size_t i = 0; i < guids.size() && i < activeSystems.size(); ++i)
        {
            if (guids[i] == _guid)
                return activeSystems[i].get();
        }
        return nullptr;
    }

    const cp::ecs::ISystem* Scene::FindActiveSystem(std::string_view _guid) const
    {
        const auto& guids = description.systemsConfig.enabledSystemGuids;
        for (size_t i = 0; i < guids.size() && i < activeSystems.size(); ++i)
        {
            if (guids[i] == _guid)
                return activeSystems[i].get();
        }
        return nullptr;
    }

    const std::vector<std::unique_ptr<cp::ecs::ISystem>>& Scene::GetActiveSystems() const
    {
        return activeSystems;
    }
}

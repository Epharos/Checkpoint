#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <Common/Core/Registry.hpp>
#include <Common/Scene/SceneDescription.hpp>

#include <ECS/System.hpp>

namespace cp::ecs
{
    class World;
}
namespace cp
{
    class FrameGraph;
    class ISerializer;
    class IDeserializer;
}

namespace cp::runtime
{
    class Scene
    {
    public:
        Scene();
        ~Scene();

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) noexcept;
        Scene& operator=(Scene&&) noexcept;

        [[nodiscard]] cp::ecs::World& GetWorld();
        [[nodiscard]] const cp::ecs::World& GetWorld() const;

        [[nodiscard]] const std::vector<std::string>& GetActivePassNames() const;
        void SetActivePassNames(std::vector<std::string> _passNames);

        [[nodiscard]] const std::vector<std::string>& GetEnabledSystemGuids() const;
        void SetEnabledSystemGuids(std::vector<std::string> _guids);

        void SetFrameGraphReference(const cp::FrameGraph* _frameGraph);
        [[nodiscard]] const cp::FrameGraph* GetFrameGraphReference() const;

        [[nodiscard]] const cp::scene::SceneDescription& GetDescription() const;
        [[nodiscard]] cp::scene::SceneDescription& GetDescription();
        [[nodiscard]] const std::string& GetName() const;
        void SetName(std::string _name);

        void Initialize(const std::string& _name);
        void Clear();

        [[nodiscard]] bool SerializeTo(cp::ISerializer& _serializer) const;
        [[nodiscard]] bool DeserializeFrom(cp::IDeserializer& _deserializer);

        /**
         * @brief Return all currently stored pass state blobs (keyed by registry type-name).
         */
        [[nodiscard]] const std::unordered_map<std::string, std::vector<std::byte>>& GetPassBlobs() const;

        /**
         * @brief Store (or replace) the serialized state for one render pass.
         * @param _typeName Registry type-name of the pass (e.g. "NegativePostFX").
         * @param _blob Serialized bytes produced by IRenderPass::Serialize().
         */
        void SetPassBlob(std::string _typeName, std::vector<std::byte> _blob);

        /**
         * @brief Remove all stored pass blobs.
         */
        void ClearPassBlobs();

        /**
         * @brief Instantiate all enabled systems from the registry.
         *
         * Any previously active systems are shut down first.
         * Pending blobs read during DeserializeFrom are applied via ISystem::Deserialize.
         *
         * @param _registry Registry to create system instances from.
         * @return Number of systems successfully initialized.
         */
        size_t InitializeSystems(const cp::Registry<cp::ecs::ISystem>& _registry);

        /**
         * @brief Destroy all active system instances.
         */
        void ShutdownSystems();

        /**
         * @brief Find a live system instance by its registry key (GUID string).
         * @return Pointer to the system, or nullptr if not found or not active.
         */
        [[nodiscard]] cp::ecs::ISystem* FindActiveSystem(std::string_view _guid);
        [[nodiscard]] const cp::ecs::ISystem* FindActiveSystem(std::string_view _guid) const;

        /**
         * @brief Access all active system instances, in the same order as enabledSystemGuids.
         */
        [[nodiscard]] const std::vector<std::unique_ptr<cp::ecs::ISystem>>& GetActiveSystems() const;

    private:
        std::unique_ptr<cp::ecs::World> world;
        const cp::FrameGraph* frameGraphRef = nullptr;
        cp::scene::SceneDescription description;

        std::vector<std::unique_ptr<cp::ecs::ISystem>> activeSystems;

        std::unordered_map<std::string, std::vector<std::byte>> pendingSystemBlobs;
    };
}

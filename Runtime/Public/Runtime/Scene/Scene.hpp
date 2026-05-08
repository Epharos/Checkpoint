#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Common/Scene/SceneDescription.hpp>

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
        [[nodiscard]] const std::string& GetName() const;
        void SetName(std::string _name);

        void Initialize(const std::string& _name);
        void Clear();

        [[nodiscard]] bool SerializeTo(cp::ISerializer& _serializer) const;
        [[nodiscard]] bool DeserializeFrom(cp::IDeserializer& _deserializer);

    private:
        std::unique_ptr<cp::ecs::World> world;
        const cp::FrameGraph* frameGraphRef = nullptr;
        cp::scene::SceneDescription description;
    };
}

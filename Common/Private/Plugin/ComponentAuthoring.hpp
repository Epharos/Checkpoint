#pragma once

#include <Common/Authoring.hpp>

#include <string_view>

namespace cp::ecs
{
	class World;
	struct Entity;
}

namespace cp
{
	class IComponentAuthoring
	{
	public:
		virtual ~IComponentAuthoring() = default;

		[[nodiscard]] virtual std::string_view Name() const = 0;
		[[nodiscard]] virtual uint64_t ComponentGuid() const = 0;

		[[nodiscard]] virtual bool HasComponent(const ecs::World& _world, const ecs::Entity& _entity) const = 0;
		[[nodiscard]] virtual bool AddComponent(ecs::World& _world, const ecs::Entity& _entity) const = 0;
		[[nodiscard]] virtual bool RemoveComponent(ecs::World& _world, const ecs::Entity& _entity) const = 0;

		[[nodiscard]] virtual std::vector<AuthoringSectionDescriptor> BuildSections(const ecs::World& _world, const ecs::Entity& _entity) const = 0;
		[[nodiscard]] virtual bool ApplyValue(
			ecs::World& _world,
			const ecs::Entity& _entity,
			std::string_view _fieldId,
			const AuthoringValue& _value,
			std::string& _outError
		) const = 0;
	};
}

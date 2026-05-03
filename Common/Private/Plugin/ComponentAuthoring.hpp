#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace cp::ecs
{
	class World;
	struct Entity;
}

namespace cp
{
	enum class AuthoringInputType : uint8_t
	{
		Default,
		FilePath
	};

	enum class AuthoringValueType : uint8_t
	{
		Bool,
		Int,
		Float,
		String,
		Vec3
	};

	struct AuthoringVec3
	{
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;
	};

	using AuthoringValue = std::variant<bool, int64_t, double, std::string, AuthoringVec3>;

	struct AuthoringFieldDescriptor
	{
		std::string id;
		std::string label;
		AuthoringValueType valueType = AuthoringValueType::String;
		AuthoringInputType inputType = AuthoringInputType::Default;
		AuthoringValue value = std::string{};
		bool readOnly = false;
	};

	struct AuthoringSectionDescriptor
	{
		std::string id;
		std::string title;
		std::vector<AuthoringFieldDescriptor> fields;
	};

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

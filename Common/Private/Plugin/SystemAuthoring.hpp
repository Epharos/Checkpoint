#pragma once

#include <Common/Authoring.hpp>
#include <ECS/TypeGuid.hpp>

#include <string_view>
#include <vector>

namespace cp::ecs
{
	class ISystem;
}

namespace cp
{
	class ISystemAuthoring
	{
	public:
		virtual ~ISystemAuthoring() = default;

		[[nodiscard]] virtual std::string_view Name() const = 0;
		[[nodiscard]] virtual ecs::TypeGuid SystemGuid() const = 0;

		/**
		 * @brief Build inspector sections reflecting the current state of the live system instance.
		 * @param _system Live system instance to read state from.
		 */
		[[nodiscard]] virtual std::vector<AuthoringSectionDescriptor> BuildSections(
			const ecs::ISystem& _system) const = 0;

		/**
		 * @brief Apply a single field edit directly to the live system instance.
		 * @param _system Live system instance to modify.
		 * @param _fieldId Field identifier (as declared in BuildSections).
		 * @param _value New value for the field.
		 * @param _outError Human-readable error message on failure.
		 * @return true on success, false on failure.
		 */
		[[nodiscard]] virtual bool ApplyValue(
			ecs::ISystem& _system,
			std::string_view _fieldId,
			const AuthoringValue& _value,
			std::string& _outError) const = 0;
	};
}

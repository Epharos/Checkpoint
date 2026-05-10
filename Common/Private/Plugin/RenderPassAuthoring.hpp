#pragma once

#include <Common/Authoring.hpp>

#include <string_view>
#include <vector>

namespace cp
{
	struct IRenderPass;
}

namespace cp
{
	class IRenderPassAuthoring
	{
	public:
		virtual ~IRenderPassAuthoring() = default;

		/** Human-readable name shown in the editor UI. */
		[[nodiscard]] virtual std::string_view Name() const = 0;

		/** Registry type-name used to look up the live pass in the Renderer. */
		[[nodiscard]] virtual std::string_view PassRegistryName() const = 0;

		/**
		 * @brief Build inspector sections reflecting the current state of the live pass instance.
		 * @param _pass Live pass instance to read state from (dynamic_cast to the concrete type).
		 */
		[[nodiscard]] virtual std::vector<AuthoringSectionDescriptor> BuildSections(
			const IRenderPass& _pass) const = 0;

		/**
		 * @brief Apply a single field edit directly to the live pass instance.
		 * @param _pass Live pass instance to modify.
		 * @param _fieldId Field identifier (as declared in BuildSections).
		 * @param _value New value for the field.
		 * @param _outError Human-readable error message on failure.
		 * @return true on success, false on failure.
		 */
		[[nodiscard]] virtual bool ApplyValue(
			IRenderPass& _pass,
			std::string_view _fieldId,
			const AuthoringValue& _value,
			std::string& _outError) const = 0;
	};
}

#pragma once

#include "ComponentTypeId.hpp"

namespace cp
{
	class IMessageVisitor;

	struct IMessageComponent
	{
		virtual ~IMessageComponent() = default;

		/**
		* @brief Returns the unique type ID of this message component.
		* 
		* @return The unique type ID of this message component.
		*/
		virtual MessageComponentTypeId GetTypeId() const = 0;
	};
}
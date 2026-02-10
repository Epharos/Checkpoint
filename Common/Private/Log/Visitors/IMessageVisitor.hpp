#pragma once

#include <vector>
#include <memory>

#include "VisitorRegistry.hpp"

namespace cp
{
	class IMessageVisitor
	{
	public:
		IMessageVisitor();

		virtual ~IMessageVisitor();

		/**
		* @brief Dispatches the given message component to the appropriate visit method registered in the VisitorRegistry.
		* 
		* @param component The message component to visit.
		* @param additionnalData Optional additional data that can be used by the visitor.
		*/
		void Dispatch(IMessageComponent& _component, void* _additionnalData = nullptr);

		/**
		* @brief Visit an unhandled message component.
		* This method is called when no visit method was found in the VisitorRegistry for the given component type.
		*
		* @param component The unhandled message component.
		* @param additionnalData Optional additional data that can be used by the visitor.
		*/
		virtual void VisitUnhandledComponent(IMessageComponent& _component, void* _additionnalData);

	protected:
		VisitorRegistry registry;
	};
}
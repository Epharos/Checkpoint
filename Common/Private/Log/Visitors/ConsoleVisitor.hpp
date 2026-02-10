#pragma once

#include "IMessageVisitor.hpp"

namespace cp
{
	class TextComponent;
	class SurroundedComponent;

	/**
	* @brief A visitor that outputs message components to the console.
	*/
	class ConsoleVisitor : public IMessageVisitor
	{
	public:
		ConsoleVisitor();

		void VisitTextComponent(const TextComponent& _component, void* _additionnalData);
		void VisitSurroundedComponent(const SurroundedComponent& _component, void* _additionnalData);
	};
}
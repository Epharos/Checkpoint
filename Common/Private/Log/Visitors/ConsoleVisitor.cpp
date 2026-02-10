#include "ConsoleVisitor.hpp"

#include <sstream>
#include <functional>

#include "../Message.hpp"

#include "../Components/TextComponent.hpp"
#include "../Components/SurroundedComponent.hpp"

namespace cp
{
	ConsoleVisitor::ConsoleVisitor()
	{
		registry.Register<TextComponent>(
			[](IMessageVisitor& _visitor, IMessageComponent& _component, void* _additionnalData) -> void {
				static_cast<ConsoleVisitor&>(_visitor).VisitTextComponent(
					static_cast<TextComponent&>(_component), 
					_additionnalData
				);
			}
		);

		registry.Register<SurroundedComponent>(
			[](IMessageVisitor& _visitor, IMessageComponent& _component, void* _additionnalData) -> void {
				static_cast<ConsoleVisitor&>(_visitor).VisitSurroundedComponent(
					static_cast<SurroundedComponent&>(_component), 
					_additionnalData
				);
			}
		);
	}

	void ConsoleVisitor::VisitTextComponent(const TextComponent& _component, void* _additionnalData)
	{
		std::stringstream* ss = static_cast<std::stringstream*>(_additionnalData);

		if(ss)
			*ss << _component.GetText();
	}

	void ConsoleVisitor::VisitSurroundedComponent(const SurroundedComponent& _component, void* _additionnalData)
	{
		std::stringstream* ss = static_cast<std::stringstream*>(_additionnalData);

		if(ss)
		{
			*ss << _component.GetPrefix();
			
			for (auto& component : _component.GetMessage().components)
			{
				Dispatch(*component, _additionnalData);
			}

			*ss << _component.GetSuffix();
		}
	}
}
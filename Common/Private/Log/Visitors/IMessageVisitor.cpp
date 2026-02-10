#include "IMessageVisitor.hpp"

namespace cp
{
	IMessageVisitor::IMessageVisitor() = default;

	IMessageVisitor::~IMessageVisitor() = default;

	void IMessageVisitor::Dispatch(IMessageComponent& _component, void* _additionnalData)
	{
		registry.Dispatch(*this, _component, _additionnalData);
	}

	void IMessageVisitor::VisitUnhandledComponent(IMessageComponent& _component, void* _additionnalData)
	{
		// Default implementation does nothing, can be overridden by derived visitors to handle unhandled components.
	}
}
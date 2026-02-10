#include "VisitorRegistry.hpp"

#include "IMessageVisitor.hpp"
#include "../Components/IMessageComponent.hpp"

namespace cp
{
	void VisitorRegistry::Dispatch(IMessageVisitor& _visitor, IMessageComponent& _component, void* _additionnalData) const
	{
		const auto it = registry.find(_component.GetTypeId());

		if (it != registry.end())
		{
			it->second(_visitor, _component, _additionnalData);
			return;
		}

		_visitor.VisitUnhandledComponent(_component, _additionnalData);
	}
}
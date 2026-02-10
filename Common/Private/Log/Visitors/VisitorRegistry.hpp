#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <functional>

#include "../Components/IMessageComponent.hpp"

namespace cp
{
	class IMessageVisitor;

	using VisitFunction = std::function<void(IMessageVisitor&, IMessageComponent&, void*)>;

	class VisitorRegistry final
	{
	private:
		std::unordered_map<MessageComponentTypeId, VisitFunction> registry;

	public:
		template<MessageComponentType T>
		void Register(VisitFunction _visitFunction)
		{
			registry[T::StaticTypeId()] = std::move(_visitFunction);
		}

		void Dispatch(IMessageVisitor& _visitor, IMessageComponent& _component, void* _additionnalData) const;
	};
}
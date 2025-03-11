#pragma once

#include "pch.hpp"
#include "../ECS/Entity/Entity.hpp"

namespace ECS
{
	class SparseSet
	{
	public:
		virtual bool Remove(Entity _entity) = 0;
		virtual bool Has(Entity _entity) const = 0;
		virtual void* GetRaw(Entity _entity) = 0;
	};
}
#pragma once

#include "pch.hpp"

namespace ECS
{
	class SparseSet
	{
	public:
		virtual bool Remove(Entity _entity) = 0;
		virtual bool Has(Entity _entity) const = 0;
	};
}
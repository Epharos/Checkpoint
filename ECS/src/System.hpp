#pragma once

#include "ComponentManager.hpp"

namespace ECS
{
	class System
	{
	public:
		virtual void Update(ComponentManager& _componentManager, const float& _dt) = 0;
	};
}
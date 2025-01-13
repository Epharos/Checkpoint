#pragma once

#include "../../pch.hpp"

class MoveSystem : public ECS::System
{
protected:
	Util::Clock internalClock;

public:
	virtual void Update(ECS::ComponentManager& _componentManager, const float& _dt);
};
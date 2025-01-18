#include "pch.hpp"

#include "MoveSystem.hpp"

void MoveSystem::Update(ECS::ComponentManager& _componentManager, const float& _dt)
{
	_componentManager.ForEachComponent<Transform>([&](Entity _entity, Transform& _transform)
		{
			float x = std::sin(internalClock.Elapsed()) * 1.0;
			float y = std::cos(internalClock.Elapsed()) * 0.2f;
			float z = std::sin(internalClock.Elapsed()) * 0.45f;

			//LOG_DEBUG("PLouf");

			_transform.Rotate(glm::vec3(_dt, 2.f * _dt, 0.4 * _dt));
		});
}
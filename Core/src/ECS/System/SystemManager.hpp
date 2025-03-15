#pragma once

#include "pch.hpp"
#include "System.hpp"

namespace cp
{
	class SystemManager
	{
	private:
		std::vector<std::unique_ptr<System>> systems;

	public:
		template <typename T, typename... Args>
		T& RegisterSystem(Args&& ... _args)
		{
			auto system = std::make_unique<T>(std::forward<Args>(_args)...);
			systems.emplace_back(std::move(system));
			return *static_cast<T*>(system);
		};

		void Update(EntityManager& _entityManager, ComponentManager& _componentManager, const float& _dt);

		void Cleanup();
	};
}
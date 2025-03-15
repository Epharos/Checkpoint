#pragma once
#include "../../pch.hpp"
#include "../../Data Structures/SparseSet.hpp"
#include "../Entity/Entity.hpp"

namespace cp
{
	template <typename T>
	class ComponentSparseSet : public SparseSet
	{
	private:
		std::vector<Entity> entities;
		std::vector<T> components;
		std::vector<int> sparse;

	public:
		bool Add(Entity _entity, T component)
		{
			if (_entity.id >= sparse.size())
			{
				sparse.resize(_entity.id + 1, -1);
			}

			if (sparse[_entity.id] == -1)
			{
				sparse[_entity.id] = entities.size();
				entities.push_back(std::move(_entity));
				components.push_back(std::move(component));
				return true;
			}

			return false;
		}

		bool Remove(Entity _entity)
		{
			if (!Has(_entity)) return false;

			int index = sparse[_entity.id];
			int lastIndex = entities.size() - 1;

			std::swap(entities[index], entities[lastIndex]);
			std::swap(components[index], components[lastIndex]);

			sparse[entities[index].id] = index;
			sparse[_entity.id] = -1;

			entities.pop_back();
			components.pop_back();
			return true;
		}

		T& Get(Entity _entity)
		{
			return components[sparse[_entity.id]];
		}

		void* GetRaw(Entity _entity) override
		{
			return &components[sparse[_entity.id]];
		}

		bool Has(Entity _entity) const
		{
			return _entity.id < sparse.size() && sparse[_entity.id] != -1;
		}

		void ForEach(std::function<void(Entity, T&)> func)
		{
			for (int i = 0; i < entities.size(); i++)
			{
				func(entities[i], components[i]);
			}
		}

		void OptimizeSparseSet()
		{
			size_t maxEntity = entities.empty() ? 0 : std::max_element(entities.begin(), entities.end(), [](const Entity& a, const Entity& b) { return a.id < b.id; })->id;
			sparse.resize(maxEntity + 1, -1);
		}

		const std::vector<Entity> GetEntities() const
		{
			return entities;
		}
	};
}
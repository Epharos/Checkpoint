#include "pch.hpp"

#include "ComponentManager.hpp"
#include "ComponentRegistry.hpp"

namespace cp
{
	void* ComponentManager::GetComponent(Entity entity, std::type_index type)
	{
		auto it = componentStorage.find(type);
		if (it == componentStorage.end()) return nullptr;
		return it->second->GetRaw(entity);
	}

	void* ComponentManager::GetComponent(Entity entity, const std::string& type)
	{
		auto it = componentStorage.find(ComponentRegistry::GetInstance().GetTypeIndex(type));
		if (it == componentStorage.end()) return nullptr;
		return it->second->GetRaw(entity);
	}
}
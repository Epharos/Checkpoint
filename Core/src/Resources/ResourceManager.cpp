#include "pch.hpp"
#include "ResourceManager.hpp"

cp::ResourceManager* cp::ResourceManager::instance = nullptr;

cp::ResourceManager* cp::ResourceManager::Create(const cp::VulkanContext& _context)
{
	if (!instance)
	{
		instance = new cp::ResourceManager(_context);
		LOG_INFO("Resource Manager created");
	}

	return instance;
}

cp::ResourceManager* cp::ResourceManager::Get()
{
	if (!instance)
	{
		throw std::runtime_error("Resource manager not created");
	}

	return instance;
}

void cp::ResourceManager::Cleanup()
{
	for (auto& resourceType : resourceTypes)
	{
		delete resourceType.second;
	}
}
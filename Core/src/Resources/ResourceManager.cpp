#include "pch.hpp"
#include "ResourceManager.hpp"

Resource::ResourceManager* Resource::ResourceManager::instance = nullptr;

Resource::ResourceManager* Resource::ResourceManager::Create(const Context::VulkanContext& _context)
{
	if (!instance)
	{
		instance = new Resource::ResourceManager(_context);
		LOG_INFO("Resource Manager created");
	}

	return instance;
}

Resource::ResourceManager* Resource::ResourceManager::Get()
{
	if (!instance)
	{
		throw std::runtime_error("Resource manager not created");
	}

	return instance;
}

void Resource::ResourceManager::Cleanup()
{
	for (auto& resourceType : resourceTypes)
	{
		delete resourceType.second;
	}
}
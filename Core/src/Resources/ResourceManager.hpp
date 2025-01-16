#pragma once

#include "../pch.hpp"
#include <typeindex>

namespace Resource
{
	class ResourceTypeBase
	{
	public:
		virtual ~ResourceTypeBase() = default;
	};

	template<class T>
	class ResourceType : public ResourceTypeBase
	{
	private:
		std::unordered_map<std::string, T*> resources;

		std::function<T* (const Context::VulkanContext& _context, const std::string&)> loadFunction = [](const Context::VulkanContext& _context, const std::string& _path)
			{ 
				LOG_WARNING(MF("Loading was not implemented for ", typeid(T).name())); 
				return nullptr;
			};

	public:
		ResourceType() = default;
		~ResourceType()
		{
			for (auto& resource : resources)
			{
				delete resource.second;
			}
		}

		T* GetResource(const std::string& name)
		{
			auto it = resources.find(name);
			if (it == resources.end())
			{
				return nullptr;
			}

			return it->second;
		}

		void AddResource(const std::string& name, T* resource)
		{
			resources[name] = resource;
		}

		void RemoveResource(const std::string& name)
		{
			auto it = resources.find(name);
			if (it != resources.end())
			{
				delete it->second;
				resources.erase(it);
			}
		}

		void SetLoader(std::function<T* (const Context::VulkanContext& _context, const std::string&)> _loadFunction)
		{
			loadFunction = _loadFunction;
		}

		T* LoadResource(const Context::VulkanContext& _context, const std::string& _name, const std::string& _path)
		{
			T* resource = loadFunction(_context, _path);
			if (resource)
			{
				resources[_name] = resource;
			}

			return resource;
		}
	};

	class ResourceManager
	{
	private:
		std::unordered_map<std::type_index, ResourceTypeBase*> resourceTypes;
		const Context::VulkanContext* context;

	public:
		ResourceManager(const Context::VulkanContext& _context) : context(&_context) {}

		void Cleanup()
		{
			for (auto& resourceType : resourceTypes)
			{
				delete resourceType.second;
			}
		}

		template<class T>
		void RegisterResourceType()
		{
			resourceTypes[typeid(T)] = new ResourceType<T>();

			std::string typeName = typeid(T).name();
			size_t nameStart = typeName.find_last_of(":");
			typeName = typeName.substr(nameStart + 1);

			LOG_TRACE(MF("Registered ", typeName, " as a resource type"));
		}

		template<class T>
		ResourceType<T>* GetResourceType()
		{
			auto it = resourceTypes.find(typeid(T));
			if (it == resourceTypes.end())
			{
				return nullptr;
			}

			//LOG_DEBUG(MF("Found resource type ", typeid(T).name()));

			return dynamic_cast<ResourceType<T>*>(it->second);
		}

		template<class T>
		T* Get(const std::string& name)
		{
			ResourceType<T>* resourceType = GetResourceType<T>();
			if (!resourceType)
			{
				throw std::runtime_error("Resource type not found");
				return nullptr;
			}

			return resourceType->GetResource(name);
		}

		template<class T>
		void Add(const std::string& name, T* resource)
		{
			ResourceType<T>* resourceType = GetResourceType<T>();
			if (!resourceType)
			{
				throw std::runtime_error("Resource type not found");
				return;
			}

			resourceType->AddResource(name, resource);
		}

		template<class T>
		void Remove(const std::string& name)
		{
			ResourceType<T>* resourceType = GetResourceType<T>();
			if (!resourceType)
			{
				throw std::runtime_error("Resource type not found");
				return;
			}

			resourceType->RemoveResource(name);
		}

		template<class T>
		T* Load(const std::string& _name, const std::string& _path)
		{
			ResourceType<T>* resourceType = GetResourceType<T>();
			if (!resourceType)
			{
				throw std::runtime_error("Resource type not found");
				return nullptr;
			}

			return resourceType->LoadResource(*context, _name, _path);
		}
	};
}
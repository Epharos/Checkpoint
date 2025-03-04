#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"
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
		std::unordered_map<std::string, std::shared_ptr<T>> resources;

#ifdef IN_EDITOR
		std::unordered_map<std::shared_ptr<T>, std::string> resourcePath;
#endif

		std::function<std::shared_ptr<T> (const Context::VulkanContext& _context, const std::string&)> loadFunction = [](const Context::VulkanContext& _context, const std::string& _path)
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
				resource.second.reset();
			}
		}

		std::shared_ptr<T> GetResource(const std::string& name)
		{
			auto it = resources.find(name);
			if (it == resources.end())
			{
				return nullptr;
			}

			return it->second;
		}

		T* GetRawResource(const std::string& name)
		{
			auto it = resources.find(name);
			if (it == resources.end())
			{
				return nullptr;
			}

			return it->second.get();
		}

#ifdef IN_EDITOR
		std::string GetResourcePath(std::shared_ptr<T> resource)
		{
			auto it = resourcePath.find(resource);

			if (it == resourcePath.end())
			{
				return "";
			}

			return it->second;
		}
#endif

		void AddResource(const std::string& name, std::shared_ptr<T> resource)
		{
			resources[name] = resource;
		}

		void AddResource(const std::string& name, T* resource)
		{
			resources[name] = std::shared_ptr<T>(resource);
		}

		void RemoveResource(const std::string& name)
		{
			auto it = resources.find(name);
			if (it != resources.end())
			{
				if (it->second.use_count() > 1)
				{
					LOG_WARNING(MF("Resource ", name, " is still in use"));
				}

				resources.erase(it);
			}
		}

		void OptimizeMemory()
		{
			for (auto it = resources.begin(); it != resources.end();)
			{
				if (it->second.use_count() == 1)
				{
					it = resources.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		void OptimizeMemory(const std::string& name)
		{
			auto it = resources.find(name);
			if (it != resources.end())
			{
				if (it->second.use_count() == 1)
				{
					resources.erase(it);
				}
			}
		}

		void SetLoader(std::function<std::shared_ptr<T>(const Context::VulkanContext& _context, const std::string&)> _loadFunction)
		{
			loadFunction = _loadFunction;
		}

		std::shared_ptr<T> LoadResource(const Context::VulkanContext& _context, const std::string& _name, const std::string& _path)
		{
			std::shared_ptr<T> resource = loadFunction(_context, _path);

			if (resource)
			{
				resources[_name] = resource;
#ifdef IN_EDITOR
				resourcePath[resource] = _path;
#endif
			}

			return resource;
		}
	};

	//TODO : MAKE THE RESOURCE MANAGER A SINGLETON, RESOURCES SHOULD BE SHARED ACROSS THE WHOLE APPLICATION

	class ResourceManager
	{
	private:
		std::unordered_map<std::type_index, ResourceTypeBase*> resourceTypes;
		const Context::VulkanContext* context;

		static ResourceManager* instance;

		ResourceManager(const Context::VulkanContext& _context) : context(&_context) {}

	public:
		NO_COPY(ResourceManager)

		static ResourceManager* Create(const Context::VulkanContext& _context);
		static ResourceManager* Get();
		void Cleanup();

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
		std::shared_ptr<T> Get(const std::string& name)
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
		std::shared_ptr<T> GetOrLoad(const std::string& _name, const std::string& _path = "")
		{
			ResourceType<T>* resourceType = GetResourceType<T>();
			if (!resourceType)
			{
				throw std::runtime_error("Resource type not found");
				return nullptr;
			}

			std::shared_ptr<T> resource = resourceType->GetResource(_name);
			if (!resource)
			{
				resource = resourceType->LoadResource(*context, _name, _path.empty() ? _name : _path);
			}

			return resource;
		}

		template<class T>
		void Add(const std::string& name, std::shared_ptr<T> resource)
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
		std::shared_ptr<T> Load(const std::string& _name, const std::string& _path)
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
#pragma once

#include "../pch.hpp"
#include "../Context/VulkanContext.hpp"
#include <typeindex>

namespace cp
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

		std::function<std::shared_ptr<T> (const cp::VulkanContext& _context, const std::string&)> loadFunction = [](const cp::VulkanContext& _context, const std::string& _path)
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

		std::string GetResourceDisplayName(std::shared_ptr<T> resource)
		{
			for (auto [name, res] : resources)
			{
				if (res.get() == resource.get())
				{
					if (name != GetResourcePath(resource))
					{
						return name;
					}

					size_t nameStart = name.find_last_of("/\\");
					return name.substr(nameStart + 1);
				}
			}
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

		void SetLoader(std::function<std::shared_ptr<T>(const cp::VulkanContext& _context, const std::string&)> _loadFunction)
		{
			loadFunction = _loadFunction;
		}

		std::shared_ptr<T> LoadResource(const cp::VulkanContext& _context, const std::string& _name, const std::string& _path)
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

	class ResourceManager
	{
	private:
		std::unordered_map<std::type_index, ResourceTypeBase*> resourceTypes;
		const cp::VulkanContext* context;

		static ResourceManager* instance;

		ResourceManager(const cp::VulkanContext& _context) : context(&_context) {}

	public:
		NO_COPY(ResourceManager)

		static ResourceManager* Create(const cp::VulkanContext& _context);
		static ResourceManager* Get();
		void Cleanup();

		template<class T>
		void RegisterResourceType()
		{
			if (resourceTypes.find(typeid(T)) != resourceTypes.end())
			{
				LOG_WARNING(MF("Resource type ", typeid(T).name(), " is already registered"));
				return;
			}

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

			return dynamic_cast<ResourceType<T>*>(it->second);
		}

		template<class T>
		bool Has(const std::string& name)
		{
			ResourceType<T>* resourceType = GetResourceType<T>();

			if (!resourceType)
			{
				throw std::runtime_error("Resource type not found");
				return false;
			}

			return resourceType->GetResource(name) != nullptr;
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

#ifdef IN_EDITOR
		template<class T>
		std::string GetResourcePath(std::shared_ptr<T> resource)
		{
			return GetResourceType<T>()->GetResourcePath(resource);
		}

		template<class T>
		std::string GetResourceDisplayName(std::shared_ptr<T> resource)
		{
			return GetResourceType<T>()->GetResourceDisplayName(resource);
		}
#endif
	};
}
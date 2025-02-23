#pragma once

#include "ComponentWidget.hpp"
#include "ComponentBase.hpp"
#include "../EntityComponentSystem.hpp"

class ComponentRegistry
{
public:
	ComponentRegistry() = default;
	NO_COPY(ComponentRegistry)

	using ComponentFactoryFunction = std::function<void(ECS::EntityComponentSystem&, Entity&)>;
	using WidgetFactoryFunction = std::function<std::unique_ptr<ComponentWidgetBase>(ECS::EntityComponentSystem&, Entity&)>;

	static ComponentRegistry& GetInstance()
	{
		static ComponentRegistry instance;
		return instance;
	}

	template<typename ComponentType, typename WidgetType>
	void Register(const std::string& _registerName)
	{
		typeIndexMap[std::type_index(typeid(ComponentType))] = _registerName;

		componentFactory[_registerName] = [](ECS::EntityComponentSystem& _ecs, Entity& _entity)
			{
				_ecs.AddComponent<ComponentType>(_entity);
			};

#ifdef IN_EDITOR
		widgetFactory[_registerName] = [](ECS::EntityComponentSystem& _ecs, Entity& _entity)
			{
				return std::make_unique<WidgetType>(_ecs.GetComponent<ComponentType>(_entity));
			};
#endif
	}

	void CreateComponent(ECS::EntityComponentSystem& _ecs, Entity& _entity, const std::string& _componentName)
	{
		auto it = componentFactory.find(_componentName);

		if (it != componentFactory.end())
		{
			it->second(_ecs, _entity);
		}
	}

	void CreateComponent(ECS::EntityComponentSystem& _ecs, Entity& _entity, const std::type_index& _typeIndex)
	{
		auto it = componentFactory.find(typeIndexMap[_typeIndex]);

		if (it != componentFactory.end())
		{
			it->second(_ecs, _entity);
		}
	}

	template<typename ComponentType>
	void CreateComponent(ECS::EntityComponentSystem& _ecs, Entity& _entity)
	{
		auto it = componentFactory.find(typeIndexMap[std::type_index(typeid(ComponentType))]);

		if (it != componentFactory.end())
		{
			it->second(_ecs, _entity);
		}
	}

#ifdef IN_EDITOR
	std::unique_ptr<ComponentWidgetBase> CreateWidget(ECS::EntityComponentSystem& _ecs, Entity& _entity, const std::string& _registerName)
	{
		auto it = widgetFactory.find(_registerName);

		if (it != widgetFactory.end())
		{
			return it->second(_ecs, _entity);
		}

		return nullptr;
	}

	std::unique_ptr<ComponentWidgetBase> CreateWidget(ECS::EntityComponentSystem& _ecs, Entity& _entity, const std::type_index& _typeIndex)
	{
		auto it = widgetFactory.find(typeIndexMap[_typeIndex]);

		if (it != widgetFactory.end())
		{
			return it->second(_ecs, _entity);
		}

		return nullptr;
	}

	std::unique_ptr<ComponentWidgetBase> CreateWidget(ECS::EntityComponentSystem& _ecs, Entity& _entity, const IComponentBase& _component)
	{
		auto it = widgetFactory.find(typeIndexMap[std::type_index(typeid(_component))]);

		if (it != widgetFactory.end())
		{
			return it->second(_ecs, _entity);
		}

		return nullptr;
	}

	template<typename ComponentType>
	std::unique_ptr<ComponentWidgetBase> CreateWidget(ECS::EntityComponentSystem& _ecs, Entity& _entity)
	{
		auto it = widgetFactory.find(typeIndexMap[std::type_index(typeid(ComponentType))]);

		if (it != widgetFactory.end())
		{
			return it->second(_ecs, _entity);
		}

		return nullptr;
	}

	std::unordered_map<std::string, WidgetFactoryFunction>& GetWidgetFactories()
	{
		return widgetFactory;
	}
#endif

	std::unordered_map<std::string, ComponentFactoryFunction>& GetComponentFactories()
	{
		return componentFactory;
	}

	std::unordered_map<std::type_index, std::string>& GetTypeIndexMap()
	{
		return typeIndexMap;
	}

private:
#ifdef IN_EDITOR
	std::unordered_map<std::string, WidgetFactoryFunction> widgetFactory;
#endif
	std::unordered_map<std::string, ComponentFactoryFunction> componentFactory;
	std::unordered_map<std::type_index, std::string> typeIndexMap;
};
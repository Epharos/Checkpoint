#pragma once

#include "ComponentWidget.hpp"
#include "IComponentSerializer.hpp"
#include "ComponentBase.hpp"
#include "../EntityComponentSystem.hpp"

namespace cp
{
	class ComponentRegistry
	{
	public:
		ComponentRegistry() = default;
		NO_COPY(ComponentRegistry)

		using ComponentFactoryFunction = std::function<bool(cp::EntityComponentSystem&, Entity&)>;
		using WidgetFactoryFunction = std::function<std::unique_ptr<ComponentWidgetBase>(cp::EntityComponentSystem&, Entity&)>;
		using SerializerFactoryFunction = std::function<std::unique_ptr<ComponentSerializerBase>(IComponentBase*&)>;

		static ComponentRegistry& GetInstance()
		{
			static ComponentRegistry instance;
			return instance;
		}

		template<typename ComponentType, typename WidgetType, typename SerializerType>
		void Register(const std::string& _registerName)
		{
			typeIndexMap[std::type_index(typeid(ComponentType))] = _registerName;

			componentFactory[_registerName] = [](cp::EntityComponentSystem& _ecs, Entity& _entity)
				{
					return _ecs.AddComponent<ComponentType>(_entity);
				};

			serializerFactory[_registerName] = [](IComponentBase*& _component)
				{
					return std::make_unique<SerializerType>(*_component);
				};

#ifdef IN_EDITOR
			widgetFactory[_registerName] = [](cp::EntityComponentSystem& _ecs, Entity& _entity)
				{
					return std::make_unique<WidgetType>(_ecs.GetComponent<ComponentType>(_entity));
				};
#endif
		}

		bool CreateComponent(cp::EntityComponentSystem& _ecs, Entity& _entity, const std::string& _componentName)
		{
			auto it = componentFactory.find(_componentName);

			if (it != componentFactory.end())
			{
				return it->second(_ecs, _entity);
			}

			return false;
		}

		bool CreateComponent(cp::EntityComponentSystem& _ecs, Entity& _entity, const std::type_index& _typeIndex)
		{
			auto it = componentFactory.find(typeIndexMap[_typeIndex]);

			if (it != componentFactory.end())
			{
				return it->second(_ecs, _entity);
			}
		}

		template<typename ComponentType>
		bool CreateComponent(cp::EntityComponentSystem& _ecs, Entity& _entity)
		{
			auto it = componentFactory.find(typeIndexMap[std::type_index(typeid(ComponentType))]);

			if (it != componentFactory.end())
			{
				return it->second(_ecs, _entity);
			}
		}

		std::unique_ptr<ComponentSerializerBase> CreateSerializer(std::type_index _componentType, IComponentBase*& _component)
		{
			auto it = serializerFactory.find(typeIndexMap[_componentType]);
			LOG_DEBUG(MF("Looking for serializer of name: ", _componentType.name(), " found: ", it != serializerFactory.end()));

			if (it != serializerFactory.end())
			{
				return it->second(_component);
			}

			return nullptr;
		}

		std::unique_ptr<ComponentSerializerBase> CreateSerializer(IComponentBase*& _component)
		{
			auto it = serializerFactory.find(typeIndexMap[std::type_index(typeid(*_component))]);
			LOG_DEBUG(MF("Looking for serializer of name: ", typeid(*_component).name(), " found: ", it != serializerFactory.end()));

			if (it != serializerFactory.end())
			{
				return it->second(_component);
			}

			return nullptr;
		}

#ifdef IN_EDITOR
		std::unique_ptr<ComponentWidgetBase> CreateWidget(cp::EntityComponentSystem& _ecs, Entity& _entity, const std::string& _registerName)
		{
			auto it = widgetFactory.find(_registerName);

			if (it != widgetFactory.end())
			{
				return it->second(_ecs, _entity);
			}

			return nullptr;
		}

		std::unique_ptr<ComponentWidgetBase> CreateWidget(cp::EntityComponentSystem& _ecs, Entity& _entity, const std::type_index& _typeIndex)
		{
			auto it = widgetFactory.find(typeIndexMap[_typeIndex]);

			if (it != widgetFactory.end())
			{
				return it->second(_ecs, _entity);
			}

			return nullptr;
		}

		std::unique_ptr<ComponentWidgetBase> CreateWidget(cp::EntityComponentSystem& _ecs, Entity& _entity, const IComponentBase& _component)
		{
			auto it = widgetFactory.find(typeIndexMap[std::type_index(typeid(_component))]);

			if (it != widgetFactory.end())
			{
				return it->second(_ecs, _entity);
			}

			return nullptr;
		}

		template<typename ComponentType>
		std::unique_ptr<ComponentWidgetBase> CreateWidget(cp::EntityComponentSystem& _ecs, Entity& _entity)
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

		std::type_index GetTypeIndex(const std::string& _typeName)
		{
			for (auto& [typeIndex, name] : typeIndexMap)
			{
				if (name == _typeName)
					return typeIndex;
			}

			return std::type_index(typeid(void));
		}

		std::string GetTypeName(const std::type_index& _typeIndex)
		{
			return typeIndexMap[_typeIndex];
		}

	private:
#ifdef IN_EDITOR
		std::unordered_map<std::string, WidgetFactoryFunction> widgetFactory;
#endif
		std::unordered_map<std::string, ComponentFactoryFunction> componentFactory;
		std::unordered_map<std::string, SerializerFactoryFunction> serializerFactory;
		std::unordered_map<std::type_index, std::string> typeIndexMap;
	};
};
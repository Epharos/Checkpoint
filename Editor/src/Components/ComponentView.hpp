#pragma once

#include "../pch.hpp"
#include "../macros.hpp"

import EditorUI;

namespace cp {
	class IComponentView {
	public:
		virtual cp::IContainer* Render(cp::IEditorUIFactory* factory) = 0;
		virtual const std::string& GetName() const = 0;
		virtual std::optional<std::string> GetIcon() const = 0;
	};

	template<ComponentBase T>
	class ComponentView : public IComponentView {
	protected:
		T* component;
		std::string name;
		std::optional<std::string> icon;
	public:
		ComponentView(T* _comp, const std::string& _name, const std::optional<std::string>& _icon = std::nullopt) : component(_comp), name(_name), icon(_icon) {}
		inline const std::string& GetName() const override { return name; }
		inline std::optional<std::string> GetIcon() const override { return icon; }
	};

	template<class C>
	concept IsComponentView = std::is_base_of<IComponentView, C>::value;

	template<ComponentBase T>
	class DefaultComponentView : public ComponentView<T> {
		public:
		DefaultComponentView(T* _comp, const std::string& _name, const std::optional<std::string>& _icon = std::nullopt) : ComponentView<T>(_comp, _name, _icon) {}
		
		virtual cp::IContainer* Render(cp::IEditorUIFactory* factory) override {
			auto container = factory->CreateContainer();

			auto label = factory->CreateLabel("Component: " + this->name);
			container->AddChild(label.release());

			return container.release();
		}
	};

	class ComponentViewRegistry {
	public:
		static ComponentViewRegistry& GetInstance() { //TODO: Make it not a singleton
			static ComponentViewRegistry instance;
			return instance;
		}

		template<ComponentBase T, IsComponentView V>
		EDITOR_API void Register(const std::string& name, const std::optional<std::string>& icon = std::nullopt) {
			factories[typeid(T).hash_code()] = [name, icon](IComponentBase* _comp) -> std::unique_ptr<IComponentView> {
				return std::unique_ptr<IComponentView>(new V(static_cast<T*>(_comp), name, icon));
			};
		}

		std::unique_ptr<IComponentView> CreateView(IComponentBase* _component) {
			auto hash = typeid(*_component).hash_code();

			if (factories.contains(hash)) {
				return factories[hash](_component);
			}

			return std::unique_ptr<IComponentView>(new DefaultComponentView<IComponentBase>(_component, typeid(*_component).name()));
		}

	protected:
		std::unordered_map<size_t, std::function<std::unique_ptr<IComponentView>(IComponentBase*)>> factories;
	};
}
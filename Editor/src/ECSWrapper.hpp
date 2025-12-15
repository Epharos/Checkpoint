#pragma once

#include "pch.hpp"
#include "Components/Transform.hpp"
#include "Components/MeshRenderer.hpp"

namespace cp {
	struct EntityAsset : public cp::ISerializable {
		std::string name;
		std::vector<EntityAsset> children;

		std::function<void(const std::string& _newName)> onNameChanged;

		bool locked = false;
		bool visible = true;
		bool favorite = false;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void AddComponent(cp::IComponentBase* _component) {
			components.push_back(_component);

			// Update shortcuts
			if (auto transformComp = dynamic_cast<Transform*>(_component)) {
				transform = transformComp;
			}

			else if (auto meshRendererComp = dynamic_cast<MeshRenderer*>(_component)) {
				meshRenderer = meshRendererComp;
			}
		}

		void SetName(const std::string& _newName)
		{
			name = _newName;
			if(onNameChanged) onNameChanged(_newName);
		}

		std::vector<cp::IComponentBase*>& GetComponents() {
			return components;
		}

		Transform* transform = nullptr; // Shortcut to Transform component
		MeshRenderer* meshRenderer = nullptr; // Shortcut to MeshRenderer component

	protected:
		std::vector<cp::IComponentBase*> components;
	};

	struct SceneAsset : public cp::ISerializable {
		std::string name;
		std::string path;
		std::vector<EntityAsset*> entities;

		cp::RendererPrototype* renderer = nullptr;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;

		void SaveScene();
	};
}
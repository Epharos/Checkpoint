#pragma once

#include "pch.hpp"
#include "Components/Transform.hpp"
#include "Components/MeshRenderer.hpp"

namespace cp {
	struct EntityAsset : public cp::ISerializable {
		std::string name;
		std::vector<EntityAsset> children;

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

		std::vector<cp::IComponentBase*>& GetComponents() {
			return components;
		}

	protected:
		std::vector<cp::IComponentBase*> components;

		Transform* transform = nullptr; // Shortcut to Transform component
		MeshRenderer* meshRenderer = nullptr; // Shortcut to MeshRenderer component
	};

	struct SceneAsset : public cp::ISerializable {
		std::string name;
		std::vector<EntityAsset*> entities;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};
}
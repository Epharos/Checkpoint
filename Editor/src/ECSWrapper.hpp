#pragma once

#include "pch.hpp"

namespace cp {
	struct EntityAsset : public cp::ISerializable {
		std::string name;
		std::vector<cp::IComponentBase*> components;
		std::vector<EntityAsset> children;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};

	struct SceneAsset : public cp::ISerializable {
		std::string name;
		std::vector<EntityAsset> entities;

		void Serialize(ISerializer& _serializer) const override;
		void Deserialize(ISerializer& _serializer) override;
	};
}
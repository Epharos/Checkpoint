module;

#include <Core.hpp>
#include <vector>

export module ECSWrapper;

export struct EntityAsset {
	std::string name;
	std::vector<cp::IComponentBase*> components;
	std::vector<EntityAsset> children;
};

export struct SceneAsset {
	std::string name;
	std::vector<EntityAsset> entities;
};
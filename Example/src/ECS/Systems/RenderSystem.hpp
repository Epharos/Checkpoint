#pragma once

#include "../../pch.hpp"

struct HashTuple : public std::hash<std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>>
{
	size_t operator()(const std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>& _tuple) const
	{
		size_t hash = 0;
		hash ^= std::hash<Resource::Material*>()(std::get<0>(_tuple)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<Resource::Mesh*>()(std::get<1>(_tuple)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<Resource::MaterialInstance*>()(std::get<2>(_tuple)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	};
};

struct CompareTuple : public std::equal_to<std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>>
{
	bool operator()(const std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>& _lhs, const std::tuple<Resource::Material*, Resource::Mesh*, Resource::MaterialInstance*>& _rhs) const
	{
		return std::get<0>(_lhs) == std::get<0>(_rhs) && std::get<1>(_lhs) == std::get<1>(_rhs) && std::get<2>(_lhs) == std::get<2>(_rhs);
	}
};

using QueryVector = std::vector<std::tuple<MeshRenderer&, Transform&>>;

class RenderSystem : public ECS::System
{
protected:
	Render::Renderer* renderer;
	QueryVector query;

	Entity renderCamera = ECS::EntityManager::NULL_ENTITY;
	vk::Buffer renderCameraBuffer;
	vk::DeviceMemory renderCameraBufferMemory;

public:
	RenderSystem(Render::Renderer* _renderer);

	void OnRegister(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager) override;

	virtual void Update(ECS::EntityManager& _entityManager, ECS::ComponentManager& _componentManager, const float& _dt);
};
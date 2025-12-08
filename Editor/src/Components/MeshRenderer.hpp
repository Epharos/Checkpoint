#pragma once

#include "../pch.hpp"

struct CheckpointEditor;

struct MeshRenderer : public cp::IComponentBase
{
	std::shared_ptr<cp::Mesh> mesh;
	std::shared_ptr<cp::MaterialInstance> materialInstance;

	class Helper : public cp::ComponentBaseHelper<MeshRenderer>
	{
		void SetMesh(MeshRenderer& _component, std::shared_ptr<cp::Mesh> _mesh)
		{
			_component.mesh = _mesh;
		}

		void SetMaterialInstance(MeshRenderer& _component, std::shared_ptr<cp::MaterialInstance> _materialInstance)
		{
			_component.materialInstance = _materialInstance;
		}
	};
};

class MeshRendererSerializer : public cp::IComponentSerializer
{
public:
	MeshRendererSerializer(cp::IComponentBase& _component) : IComponentSerializer(_component) {}

	void Serialize(cp::ISerializer& _serializer) const override;

	void Deserialize(cp::ISerializer& _serializer) override;
};

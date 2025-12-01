#pragma once

#include "../pch.hpp"
#include "../CheckpointEditor.hpp"

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

	void Serialize(cp::ISerializer& _serializer) const override
	{
		MeshRenderer& component = static_cast<MeshRenderer&>(this->component);
		
		std::string meshRelativePath = cp::CheckpointEditor::CurrentProject.GetResourceRelativePath(cp::ResourceManager::Get()->GetResourceType<cp::Mesh>()->GetResourcePath(component.mesh));
		_serializer.WriteString("mesh", meshRelativePath);
	}

	void Deserialize(cp::ISerializer& _serializer) override
	{
		MeshRenderer& component = static_cast<MeshRenderer&>(this->component);
		std::string fullMeshPath = cp::CheckpointEditor::CurrentProject.GetResourcePath() + "/" + _serializer.ReadString("mesh", "");
		if(!fullMeshPath.empty()) component.mesh = cp::ResourceManager::Get()->GetOrLoad<cp::Mesh>(fullMeshPath);
	}
};

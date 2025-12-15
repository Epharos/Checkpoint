#include "pch.hpp"

#include "MeshRenderer.hpp"
#include "../CheckpointEditor.hpp"

void MeshRendererSerializer::Serialize(cp::ISerializer& _serializer) const
{
	const MeshRenderer& component = static_cast<const MeshRenderer&>(this->component);

	std::string meshRelativePath = cp::CheckpointEditor::CurrentProject.GetResourceRelativePath(cp::ResourceManager::Get()->GetResourceType<cp::Mesh>()->GetResourcePath(component.mesh));
	_serializer.WriteString("mesh", meshRelativePath);

	std::string materialInstanceRelativePath = cp::CheckpointEditor::CurrentProject.GetResourceRelativePath(cp::ResourceManager::Get()->GetResourceType<cp::MaterialInstance>()->GetResourcePath(component.materialInstance));
	_serializer.WriteString("materialInstance", materialInstanceRelativePath);
}

void MeshRendererSerializer::Deserialize(cp::ISerializer& _serializer)
{
	MeshRenderer& component = static_cast<MeshRenderer&>(this->component);
	std::string fullMeshPath = cp::CheckpointEditor::CurrentProject.GetResourcePath() + "/" + _serializer.ReadString("mesh", "");
	if (!fullMeshPath.empty()) component.mesh = cp::ResourceManager::Get()->GetOrLoad<cp::Mesh>(fullMeshPath);

	std::string fullMaterialInstancePath = cp::CheckpointEditor::CurrentProject.GetResourcePath() + "/" + _serializer.ReadString("materialInstance", "");
	if (!fullMaterialInstancePath.empty()) component.materialInstance = cp::ResourceManager::Get()->GetOrLoad<cp::MaterialInstance>(fullMaterialInstancePath);
}
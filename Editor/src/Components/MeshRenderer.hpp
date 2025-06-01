#pragma once

#include "../pch.hpp"

struct MeshRenderer : public cp::IComponentBase
{
	std::shared_ptr<cp::Mesh> mesh;
	std::shared_ptr<cp::MaterialInstance> materialInstance;

	static class Helper : public cp::ComponentBaseHelper<MeshRenderer>
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
		std::string meshRelativePath = cp::Project::GetResourceRelativePath(cp::ResourceManager::Get()->GetResourceType<cp::Mesh>()->GetResourcePath(component.mesh));
		_serializer.WriteString("mesh", meshRelativePath);
	}

	void Deserialize(cp::ISerializer& _serializer) override
	{
		MeshRenderer& component = static_cast<MeshRenderer&>(this->component);
		std::string fullMeshPath = cp::Project::GetResourcePath() + "/" + _serializer.ReadString("mesh", "");
		if(!fullMeshPath.empty()) component.mesh = cp::ResourceManager::Get()->GetOrLoad<cp::Mesh>(fullMeshPath);
	}
};

class MeshRendererWidget : public cp::ComponentWidget<MeshRenderer>
{
public:
	MeshRendererWidget(MeshRenderer& _component, QWidget* _parent = nullptr) : ComponentWidget(_component, "Mesh Renderer", _parent)
	{

	}

	void Initialize() override
	{
		QLabel* positionLabel = new QLabel("Mesh", this);
		layout->addWidget(positionLabel);

		cp::MeshDropLineEdit* meshLineEdit = new cp::MeshDropLineEdit(true, this);
		meshLineEdit->SetResource(&component.mesh);

		layout->addWidget(meshLineEdit);
	}
};
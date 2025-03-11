#pragma once

#include "../pch.hpp"

#include "../Widgets/ComponentFields/FileDropLineEdit.hpp"

struct MeshRenderer : public IComponentBase
{
	std::shared_ptr<Resource::Mesh> mesh;
	std::shared_ptr<Resource::MaterialInstance> materialInstance;

	static class Helper : public ComponentBaseHelper<MeshRenderer>
	{
		void SetMesh(MeshRenderer& _component, std::shared_ptr<Resource::Mesh> _mesh)
		{
			_component.mesh = _mesh;
		}

		void SetMaterialInstance(MeshRenderer& _component, std::shared_ptr<Resource::MaterialInstance> _materialInstance)
		{
			_component.materialInstance = _materialInstance;
		}
	};
};

class MeshRendererSerializer : public IComponentSerializer<MeshRenderer>
{
	void Serialize(Serializer& _serializer) const override
	{
		std::string meshRelativePath = Project::GetResourceRelativePath(Resource::ResourceManager::Get()->GetResourceType<Resource::Mesh>()->GetResourcePath(component.mesh));
		_serializer.WriteString("mesh", meshRelativePath);
	}

	void Deserialize(Serializer& _serializer) override
	{
		std::string fullMeshPath = Project::GetResourcePath() + "/" + _serializer.ReadString("mesh", "");
		if(!fullMeshPath.empty()) component.mesh = Resource::ResourceManager::Get()->GetOrLoad<Resource::Mesh>(fullMeshPath);
	}
};

class MeshRendererWidget : public ComponentWidget<MeshRenderer>
{
public:
	MeshRendererWidget(MeshRenderer& _component, QWidget* _parent = nullptr) : ComponentWidget(_component, "Mesh Renderer", _parent)
	{

	}

	void Initialize() override
	{
		QLabel* positionLabel = new QLabel("Mesh", this);
		layout->addWidget(positionLabel);

		MeshDropLineEdit* meshLineEdit = new MeshDropLineEdit(this);
		meshLineEdit->SetResource(&component.mesh);

		layout->addWidget(meshLineEdit);
	}
};
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
	virtual QJsonObject Serialize(const MeshRenderer& _component)
	{
		QJsonObject obj;

		obj["mesh"] = QString::fromStdString(Resource::ResourceManager::Get()->GetResourceType<Resource::Mesh>()->GetResourcePath(_component.mesh));

		return obj;
	}

	virtual void Deserialize(const QJsonObject& _data, MeshRenderer& _component)
	{
		_component.mesh = Resource::ResourceManager::Get()->GetOrLoad<Resource::Mesh>(_data["mesh"].toString().toStdString());
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
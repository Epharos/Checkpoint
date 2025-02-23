#pragma once

#include "../pch.hpp"

struct MeshRenderer : public IComponentBase
{
	Resource::Mesh* mesh = nullptr;
	Resource::MaterialInstance* materialInstance = nullptr;

	static class Helper : public ComponentBaseHelper<MeshRenderer>
	{
		void SetMesh(MeshRenderer& _component, Resource::Mesh* _mesh)
		{
			_component.mesh = _mesh;
		}

		void SetMaterialInstance(MeshRenderer& _component, Resource::MaterialInstance* _materialInstance)
		{
			_component.materialInstance = _materialInstance;
		}

		//TODO : Unload mesh and material instance if they are not used by other entities
	};
};

class MeshRendererWidget : public ComponentWidget<MeshRenderer>
{
public:
	MeshRendererWidget(MeshRenderer& _component, QWidget* _parent = nullptr) : ComponentWidget(_component, "Mesh Renderer", _parent)
	{

	}

	void Initialize() override
	{
		QLabel* positionLabel = new QLabel("Plouf", this);
		layout->addWidget(positionLabel);
	}
};
#include "pch.hpp"
#include "CheckpointEditor.hpp"

#include <QtGui/qfontdatabase.h>

#include "Components/Transform.hpp"
#include "Components/MeshRenderer.hpp"

#include "ECSWrapper.hpp"

#include "EditorUI/EditorWindow.hpp"
#include "EditorUI/Launcher.hpp"

#include "Components/ComponentView.hpp"
#include "Renderers/EditorRenderer.hpp"

import EditorUI;

class TransformView : public cp::ComponentView<Transform> {
	public:
	TransformView(Transform* _comp, const std::string& _name, const std::optional<std::string>& _icon = std::nullopt) : cp::ComponentView<Transform>(_comp, _name, _icon) {}

	virtual cp::IContainer* Render(cp::IEditorUIFactory* factory) override {
		auto container = factory->CreateContainer();
		auto positionWidget = factory->CreateFloat3Field(&component->position, "Position");
		container->AddChild(positionWidget.release());
		auto rotationWidget = factory->CreateQuaternionField(&component->rotation, "Rotation");
		container->AddChild(rotationWidget.release());
		auto scaleWidget = factory->CreateFloat3Field(&component->scale, "Scale");
		container->AddChild(scaleWidget.release());
		container->SetSpacing(2);
		return container.release();
	}
};

class MeshRendererView : public cp::ComponentView<MeshRenderer> {
	public:
	MeshRendererView(MeshRenderer* _comp, const std::string& _name, const std::optional<std::string>& _icon = std::nullopt) : cp::ComponentView<MeshRenderer>(_comp, _name, _icon) {}
	virtual cp::IContainer* Render(cp::IEditorUIFactory* factory) override {
		auto container = factory->CreateContainer();
		auto meshSelector = factory->CreateMeshSelector(&component->mesh, "Mesh");
		container->AddChild(meshSelector.release());
		auto materialSelector = factory->CreateMaterialInstanceSelector(&component->materialInstance, "Material Instance");
		container->AddChild(materialSelector.release());
		container->SetSpacing(2);
		return container.release();
	}
protected:
	std::string meshPath, materialPath;
};

QString LoadStyleSheet(const QString& path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
	{
		std::cerr << "Failed to open file: " << path.toStdString() << std::endl;
		return "";
	}

	QString styleSheet = file.readAll();
	file.close();

	return styleSheet;
}

int main(int argc, char* args[])
{
	QApplication app(argc, args);

	cp::ComponentRegistry::GetInstance().Register<Transform, TransformSerializer>("Transform");
	cp::ComponentRegistry::GetInstance().Register<MeshRenderer, MeshRendererSerializer>("Mesh Renderer");

	cp::ComponentViewRegistry::GetInstance().Register<Transform, TransformView>("Transform");
	cp::ComponentViewRegistry::GetInstance().Register<MeshRenderer, MeshRendererView>("Mesh Renderer");

	app.setStyleSheet(LoadStyleSheet("Editor_Resources/Stylesheet.qss"));

	int fontID = QFontDatabase::addApplicationFont("Editor_Resources/Montserrat.ttf");

	fontID = QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Medium.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-MediumItalic.ttf");
	fontID = QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Book.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-BookItalic.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Bold.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-BoldItalic.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-Black.ttf");
	QFontDatabase::addApplicationFont("Editor_Resources/CircularStd/CircularStd-BlackItalic.ttf");

	QStringList families = QFontDatabase::applicationFontFamilies(fontID);
	qDebug() << families;

	if (fontID != -1)
	{
		QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontID);

		if (!fontFamilies.isEmpty())
		{
			QFont font(fontFamilies.first(), 10);
			font.setStyleStrategy(QFont::PreferQuality);
			font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
			app.setFont(font);
		}
		else
		{
			std::cerr << "Failed to load font." << std::endl;
		}
	}
	else
	{
		std::cerr << "Failed to load font." << std::endl;
	}

	cp::CheckpointEditor::SetupVulkanContext();

	cp::ResourceManager::Create(cp::CheckpointEditor::VulkanCtx);
	cp::ResourceManager::Get()->RegisterResourceType<cp::Mesh>();
	cp::ResourceManager::Get()->GetResourceType<cp::Mesh>()->SetLoader(std::bind(&cp::Mesh::LoadMesh, std::placeholders::_1, std::placeholders::_2));
	cp::ResourceManager::Get()->RegisterResourceType<cp::Texture>();
	cp::ResourceManager::Get()->GetResourceType<cp::Texture>()->SetLoader(std::bind(&cp::Texture::LoadTexture, std::placeholders::_1, std::placeholders::_2));
	cp::ResourceManager::Get()->RegisterResourceType<cp::Material>();
	cp::ResourceManager::Get()->GetResourceType<cp::Material>()->SetLoader(std::bind(&cp::Material::LoadMaterial, std::placeholders::_1, std::placeholders::_2));
	cp::ResourceManager::Get()->RegisterResourceType<cp::MaterialInstance>();
	cp::ResourceManager::Get()->GetResourceType<cp::MaterialInstance>()->SetLoader(std::bind(&cp::MaterialInstance::LoadMaterialInstance, std::placeholders::_1, std::placeholders::_2));

	cp::Launcher launcher;

	return app.exec();
}
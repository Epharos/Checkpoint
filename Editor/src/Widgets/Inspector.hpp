#pragma once

#include "../pch.hpp"
#include "SearchList.hpp"

#include "Widgets/ComponentFields/String.hpp"
#include "Widgets/ComponentFields/FileDropLineEdit.hpp"
#include "Widgets/Collapsible.hpp"

#include <QtWidgets/qcheckbox.h>

class Inspector : public QWidget
{
	Q_OBJECT

protected:
	QVBoxLayout* layout;
	QLabel* titleLabel;
	cp::Scene* scene = nullptr;

	void* readFile = nullptr;

	std::unordered_map<std::string, std::function<void(const std::string&, const QFileInfo&)>> fileInspector;

public:
	Inspector(cp::Scene* _scene, QWidget* _parent = nullptr) : QWidget(_parent), scene(_scene)
	{
		layout = new QVBoxLayout(this);
		titleLabel = new QLabel("Select an entity or a file", this);
		layout->addWidget(titleLabel);
		setLayout(layout);
		layout->setAlignment(Qt::AlignTop);

		setBaseSize(400, 960);

		setMinimumWidth(350);
		setMinimumHeight(480);

		fileInspector["mat"] = [=](const std::string& _path, const QFileInfo& _fileInfo) { ShowMaterial(_path, _fileInfo); };
	}

	void CreateAddComponentButton(cp::Entity* _entity)
	{
		QPushButton* addComponentButton = new QPushButton("Add Component", this);
		layout->addWidget(addComponentButton);

		connect(addComponentButton, &QPushButton::clicked, [=] {
			QMenu* menu = new QMenu(addComponentButton);

			SearchList* searchList = new SearchList(menu);
			searchList->Populate(cp::ComponentRegistry::GetInstance().GetTypeIndexMap());

			QWidgetAction* widgetAction = new QWidgetAction(menu);
			widgetAction->setDefaultWidget(searchList);
			menu->addAction(widgetAction);

			connect(searchList, &SearchList::ItemSelected, [=](std::type_index _type) {
				cp::ComponentRegistry::GetInstance().CreateComponent(scene->GetECS(), *_entity, _type);
				UpdateComponents(_entity);
				if (menu) menu->close();
				});

			menu->popup(addComponentButton->mapToGlobal(QPoint(0, addComponentButton->height())));
			});
	}

	void UpdateComponents(cp::Entity* _entity)
	{
		QLayoutItem* child;
		while ((child = layout->takeAt(2)) != nullptr)
		{
			if(child->widget()) child->widget()->deleteLater();
			else if (child->layout()) child->layout()->deleteLater();
			else delete child;
		}

		for (auto& [type, component] : scene->GetECS().GetAllComponentsOf(*_entity))
		{
			auto widget = cp::ComponentRegistry::GetInstance().CreateWidget(scene->GetECS(), *_entity, type);
			widget->Initialize();
			layout->addWidget(widget.release());
			layout->addSpacerItem(new QSpacerItem(0, 10));
		}

		CreateAddComponentButton(_entity);

		layout->update();
	}

	void ShowEntity(cp::Entity* _entity)
	{
		if (readFile)
		{
			delete readFile;
			readFile = nullptr;
		}

		titleLabel->setText(QString::fromStdString(_entity->GetDisplayName()));

		layout->addSpacerItem(new QSpacerItem(0, 10));

		UpdateComponents(_entity);
	}

	void ShowFile(const std::string& _path, const QFileInfo& _fileInfo)
	{
		QString fileName = QString::fromStdString(Project::GetResourceRelativePath(_path));

		titleLabel->setText(fileName);

		if(readFile)
		{
			delete readFile;
			readFile = nullptr;
		}

		QLayoutItem* child;
		while ((child = layout->takeAt(1)) != nullptr)
		{
			if(child->widget()) child->widget()->deleteLater();
			else if(child->layout()) child->layout()->deleteLater();
			else delete child;
		}

		if (fileInspector.find(_fileInfo.suffix().toStdString()) != fileInspector.end())
		{
			fileInspector[_fileInfo.suffix().toStdString()](_path, _fileInfo);
		}
		else
		{
			QLabel* label = new QLabel("No inspector available for this file type", this);
			layout->addWidget(label);
		}

		layout->update();
	}

	void ShowMaterial(const std::string& _path, const QFileInfo& _fileInfo)
	{
		readFile = new cp::Material(scene->GetRenderer()->GetContext());
		cp::Material* mat = static_cast<cp::Material*>(readFile);
		cp::JsonSerializer serializer;
		serializer.Read(_path);
		mat->Deserialize(serializer);

		Collapsible* properties = new Collapsible("Properties", this);
		layout->addWidget(properties);

		String* nameMat = new String(mat->GetNamePtr(), "Material name", this);
		properties->AddWidget(nameMat);

		QLabel* shaderLabel = new QLabel("Shader path", this);
		FileDropLineEdit* shaderPath = new FileDropLineEdit(this);
		shaderPath->SetResourcePath(mat->GetShaderPath());
		shaderPath->SetAcceptedExtensions({ "slang" });
		properties->AddWidget(shaderLabel);
		properties->AddWidget(shaderPath);

		Collapsible* shaderStages = new Collapsible("Shader stages", this, false);
		properties->AddWidget(shaderStages);

		QCheckBox* vertexCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::Vertex, "Vertex");
		QCheckBox* fragmentCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::Fragment, "Fragment");
		QCheckBox* geometryCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::Geometry, "Geometry");
		QCheckBox* tessellationControlCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::TessellationControl, "Tessellation control");
		QCheckBox* tessellationEvaluationCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::TessellationEvaluation, "Tessellation evaluation");
		QCheckBox* meshCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::Mesh, "Mesh (exp)");
		QCheckBox* computeCheckBox = CreateShaderStagePicker(*mat, cp::ShaderStages::Compute, "Compute");

		shaderStages->AddWidget(vertexCheckBox);
		shaderStages->AddWidget(fragmentCheckBox);
		shaderStages->AddWidget(geometryCheckBox);
		shaderStages->AddWidget(tessellationControlCheckBox);
		shaderStages->AddWidget(tessellationEvaluationCheckBox);
		shaderStages->AddWidget(meshCheckBox);

		Collapsible* renderpasses = new Collapsible("Render passes", this);
		layout->addWidget(renderpasses);

		for (auto& rp : scene->GetRenderer()->GetRenderPassNames())
		{
			QCheckBox* renderActive = new QCheckBox(QString::fromStdString(rp), this);
			renderActive->setChecked(mat->GetRenderPassRequirement(rp).renderToPass);
			renderpasses->AddWidget(renderActive);

			QLineEdit* customShaderPath = new QLineEdit(this);
			customShaderPath->setPlaceholderText(QString::fromStdString(shaderPath->GetRelativeResourcePath()));
			customShaderPath->setText(QString::fromStdString(mat->GetRenderPassRequirement(rp).customShaderPath));
			customShaderPath->setVisible(renderActive->isChecked());
			renderpasses->AddWidget(customShaderPath);

			Collapsible* entryPoints = new Collapsible("Entry points", this, false);
			entryPoints->setVisible(renderActive->isChecked());
			renderpasses->AddWidget(entryPoints);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Vertex, "Vertex_Default", rp, vertexCheckBox))
				entryPoints->AddWidget(entryPoint);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Fragment, "Fragment_Default", rp, fragmentCheckBox))
				entryPoints->AddWidget(entryPoint);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Geometry, "Geometry_Default", rp, geometryCheckBox))
				entryPoints->AddWidget(entryPoint);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::TessellationControl, "TessellationControl_Default", rp, tessellationControlCheckBox))
				entryPoints->AddWidget(entryPoint);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::TessellationEvaluation, "TessellationEvaluation_Default", rp, tessellationEvaluationCheckBox))
				entryPoints->AddWidget(entryPoint);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Mesh, "Mesh_Default", rp, meshCheckBox))
				entryPoints->AddWidget(entryPoint);

			if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Compute, "Compute_Default", rp, computeCheckBox))
				entryPoints->AddWidget(entryPoint);

			renderpasses->AddSpacing(10);

			connect(renderActive, &QCheckBox::checkStateChanged, [=](Qt::CheckState _state) {
					mat->GetRenderPassRequirement(rp).renderToPass = _state == Qt::CheckState::Checked;
					customShaderPath->setVisible(_state == Qt::CheckState::Checked);
					entryPoints->setVisible(_state == Qt::CheckState::Checked);
				});
		}

		QPushButton* saveButton = new QPushButton("Save", this);
		layout->addWidget(saveButton);

		connect(shaderPath, &FileDropLineEdit::ResourcePathChanged, [=](const std::string& _path) {
			mat->SetShaderPath(_path);
			});

		connect(saveButton, &QPushButton::clicked, [=] {
			cp::JsonSerializer serializer;
			mat->Serialize(serializer);
			serializer.Write(_path);
			});
	}

	QLineEdit* CreateEntryPointOverride(cp::Material& _material, cp::ShaderStages _stage, const std::string& _placeholder, const std::string& _renderPass, const QCheckBox* _stageActive)
	{
		if (!_material.HasShaderStage(_stage)) return nullptr;

		QLineEdit* entryPoint = new QLineEdit(this);
		entryPoint->setPlaceholderText(QString::fromStdString(_placeholder));

		if(_material.GetRenderPassRequirement(_renderPass).customEntryPoints.find(_stage) != _material.GetRenderPassRequirement(_renderPass).customEntryPoints.end())
			entryPoint->setText(QString::fromStdString(_material.GetRenderPassRequirement(_renderPass).customEntryPoints.at(_stage)));

		return entryPoint;
	}

	QCheckBox* CreateShaderStagePicker(cp::Material& _material, cp::ShaderStages _stage, const std::string& _stageName)
	{
		QCheckBox* checkBox = new QCheckBox(QString::fromStdString(_stageName), this);
		checkBox->setChecked(_material.HasShaderStage(_stage));

		connect(checkBox, &QCheckBox::checkStateChanged, [&_material, _stage](Qt::CheckState _state) {
			if (_state == Qt::CheckState::Checked)
				_material.AddShaderStage(_stage);
			else
				_material.RemoveShaderStage(_stage);
			});

		return checkBox;
	}
};
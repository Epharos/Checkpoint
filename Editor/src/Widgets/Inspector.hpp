#pragma once

#include "../pch.hpp"
#include "SearchList.hpp"

#include "Widgets/ComponentFields/FileDropLineEdit.hpp"
#include "Widgets/Collapsible.hpp"
#include "Widgets/EnumMultiSelectDropDown.hpp"

#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qscrollarea.h>

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
		QScrollArea* scrollArea = new QScrollArea();
		scrollArea->setWidgetResizable(true);
		scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		setLayout(mainLayout);

		QWidget* content = new QWidget();
		layout = new QVBoxLayout(content);
		scrollArea->setWidget(content);
		
		mainLayout->addWidget(scrollArea);

		titleLabel = new QLabel("Select an entity or a file", this);
		layout->addWidget(titleLabel);
		layout->setAlignment(Qt::AlignTop);

		setBaseSize(400, 960);

		setMinimumWidth(350);
		setMinimumHeight(480);

		fileInspector["mat"] = [=](const std::string& _path, const QFileInfo& _fileInfo) { ShowMaterial(_path, _fileInfo); };
		fileInspector["matinstance"] = [=](const std::string& _path, const QFileInfo& _fileInfo) { ShowMaterialInstance(_path, _fileInfo); };
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

#pragma region Material Properties
		Collapsible* properties = new Collapsible("Properties", this);
		layout->addWidget(properties);

		cp::String* nameMat = new cp::String(mat->GetNamePtr(), "Material name", this);
		properties->AddWidget(nameMat);

		QLabel* shaderLabel = new QLabel("Shader path", this);
		FileDropLineEdit* shaderPath = new FileDropLineEdit(this);
		shaderPath->SetResourcePath(mat->GetShaderPath());
		shaderPath->SetAcceptedExtensions({ "slang", "spv" });
		properties->AddWidget(shaderLabel);
		properties->AddWidget(shaderPath);

#ifdef _DEBUG
		std::set<cp::ShaderStages> uniqueShaderStages;

		for (const auto& [_, stage] : mat->GetShaderReflection()->entryPoints)
		{
			uniqueShaderStages.insert(stage);
		}

		if (uniqueShaderStages.size() >= 1)
		{
			QLabel* entryPointsLabel = new QLabel("Shader stages [debug]", this);
			properties->AddWidget(entryPointsLabel);

			for (const auto& stage : uniqueShaderStages)
			{
				QLabel* shaderStage = new QLabel(this);
				shaderStage->setText(QString::fromStdString(Helper::Material::GetShaderStageString(stage)));
				shaderStage->setStyleSheet("color : #aaa;");
				properties->AddWidget(shaderStage);
			}
		}
#endif

		/*CheckableComboBox* shaderStageComboBox = new CheckableComboBox(this);
		shaderStageComboBox->setMinimumHeight(30);
		shaderStageComboBox->setMaximumHeight(30);
		shaderStageComboBox->AddCheckItem(QString::fromStdString(Helper::Material::GetShaderStageString(cp::ShaderStages::Vertex)), mat->HasShaderStage(cp::ShaderStages::Vertex) ? Qt::Checked : Qt::Unchecked);
		shaderStageComboBox->AddCheckItem(QString::fromStdString(Helper::Material::GetShaderStageString(cp::ShaderStages::Fragment)), mat->HasShaderStage(cp::ShaderStages::Fragment) ? Qt::Checked : Qt::Unchecked);
		shaderStageComboBox->AddCheckItem(QString::fromStdString(Helper::Material::GetShaderStageString(cp::ShaderStages::Geometry)), mat->HasShaderStage(cp::ShaderStages::Geometry) ? Qt::Checked : Qt::Unchecked);
		shaderStageComboBox->AddCheckItem(QString::fromStdString(Helper::Material::GetShaderStageString(cp::ShaderStages::TessellationControl)), mat->HasShaderStage(cp::ShaderStages::TessellationControl) ? Qt::Checked : Qt::Unchecked);
		shaderStageComboBox->AddCheckItem(QString::fromStdString(Helper::Material::GetShaderStageString(cp::ShaderStages::TessellationEvaluation)), mat->HasShaderStage(cp::ShaderStages::TessellationEvaluation) ? Qt::Checked : Qt::Unchecked);
		shaderStageComboBox->AddCheckItem(QString::fromStdString(Helper::Material::GetShaderStageString(cp::ShaderStages::Mesh)), mat->HasShaderStage(cp::ShaderStages::Mesh) ? Qt::Checked : Qt::Unchecked);
		properties->AddWidget(shaderStageComboBox);*/

		connect(shaderPath, &FileDropLineEdit::ResourcePathChanged, [=](const std::string& _path) {
			mat->SetShaderPath(_path);
			});
#pragma endregion

#pragma region Render passes settings
		Collapsible* renderpasses = new Collapsible("Render passes", this);
		layout->addWidget(renderpasses);

		for (auto& [name, rp] : scene->GetRenderer()->GetRenderPasses())
		{
			QCheckBox* renderActive = new QCheckBox(QString::fromStdString(name), this);
			renderActive->setChecked(mat->GetRenderPassRequirement(name).renderToPass);
			renderpasses->AddWidget(renderActive);

			QCheckBox* useDefaultShader = new QCheckBox("Use default shader", this);
			useDefaultShader->setChecked(mat->GetRenderPassRequirement(name).useDefaultShader);
			useDefaultShader->setVisible(renderActive->isChecked() && rp.GetDefaultPipeline().has_value());
			useDefaultShader->setStyleSheet("margin-left: 10px;");
			renderpasses->AddWidget(useDefaultShader);

			QLabel* customShaderLabel = new QLabel("or", this);
			customShaderLabel->setVisible(renderActive->isChecked() && !useDefaultShader->isChecked());
			customShaderLabel->setAlignment(Qt::AlignCenter);
			renderpasses->AddWidget(customShaderLabel);

			QLineEdit* customShaderPath = new QLineEdit(this);
			//customShaderPath->setPlaceholderText(QString::fromStdString(shaderPath->GetRelativeResourcePath()));
			customShaderPath->setText(QString::fromStdString(mat->GetRenderPassRequirement(name).customShaderPath));
			customShaderPath->setVisible(renderActive->isChecked() && (!useDefaultShader->isChecked() || !rp.GetDefaultPipeline().has_value()));
			customShaderPath->setStyleSheet("margin-left: 10px;");
			renderpasses->AddWidget(customShaderPath);

			Collapsible* entryPoints = new Collapsible("Entry points", this, false);
			entryPoints->setVisible(renderActive->isChecked() && (!useDefaultShader->isChecked() || !rp.GetDefaultPipeline().has_value()));
			entryPoints->setStyleSheet("margin-left: 10px;");
			renderpasses->AddWidget(entryPoints);

			/*if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Vertex, "Vertex_Default", name))
				entryPoints->AddWidget(entryPoint);*/

			if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::Vertex, "Vertex_Default", name))
				entryPoints->AddWidget(entryPoint);

			//if (!rp.IsDepthOnly()) // Fragment shader is available only if the renderpass is not depth only
			//	if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Fragment, "Fragment_Default", name))
			//		entryPoints->AddWidget(entryPoint);

			if (!rp.IsDepthOnly()) // Fragment shader is available only if the renderpass is not depth only
				if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::Fragment, "Fragment_Default", name))
					entryPoints->AddWidget(entryPoint);

			/*if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Geometry, "Geometry_Default", name))
				entryPoints->AddWidget(entryPoint);*/

			if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::Geometry, "Geometry_Default", name))
				entryPoints->AddWidget(entryPoint);

			/*if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::TessellationControl, "TessellationControl_Default", name))
				entryPoints->AddWidget(entryPoint);*/

			if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::TessellationControl, "TessellationControl_Default", name))
				entryPoints->AddWidget(entryPoint);

			/*if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::TessellationEvaluation, "TessellationEvaluation_Default", name))
				entryPoints->AddWidget(entryPoint);*/

			if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::TessellationEvaluation, "TessellationEvaluation_Default", name))
				entryPoints->AddWidget(entryPoint);

			/*if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Mesh, "Mesh_Default", name))
				entryPoints->AddWidget(entryPoint);*/

			if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::Mesh, "Mesh_Default", name))
				entryPoints->AddWidget(entryPoint);

			/*if (QLineEdit* entryPoint = CreateEntryPointOverride(*mat, cp::ShaderStages::Compute, "Compute_Default", name))
				entryPoints->AddWidget(entryPoint);*/

			if (QComboBox* entryPoint = CreateEntryPointOverrideComboBox(*mat, cp::ShaderStages::Compute, "Compute_Default", name))
				entryPoints->AddWidget(entryPoint);

			renderpasses->AddSpacing(10);

			connect(renderActive, &QCheckBox::checkStateChanged, [=](Qt::CheckState _state) {
				mat->GetRenderPassRequirement(name).renderToPass = _state == Qt::CheckState::Checked;
				customShaderPath->setVisible(_state == Qt::CheckState::Checked && (!useDefaultShader->isChecked() || !rp.GetDefaultPipeline().has_value()));
				entryPoints->setVisible(_state == Qt::CheckState::Checked && (!useDefaultShader->isChecked() || !rp.GetDefaultPipeline().has_value()));
				customShaderLabel->setVisible(_state == Qt::CheckState::Checked && rp.GetDefaultPipeline().has_value() && !useDefaultShader->isChecked());
				useDefaultShader->setVisible(_state == Qt::CheckState::Checked && rp.GetDefaultPipeline().has_value());
				});

			connect(useDefaultShader, &QCheckBox::checkStateChanged, [=](Qt::CheckState _state) {
				mat->GetRenderPassRequirement(name).useDefaultShader = _state == Qt::CheckState::Checked;
				customShaderPath->setVisible(_state == Qt::CheckState::Unchecked);
				entryPoints->setVisible(_state == Qt::CheckState::Unchecked);
				customShaderLabel->setVisible(_state == Qt::CheckState::Unchecked);
				});
		}
#pragma endregion

#pragma region Buffers
		Collapsible* buffers = new Collapsible("Buffers data", this);
		layout->addWidget(buffers);

		std::sort(mat->GetShaderReflection()->resources.begin(), mat->GetShaderReflection()->resources.end(), [](const cp::ShaderResource& a, const cp::ShaderResource& b) {
			return a.set < b.set && a.binding < b.binding;
			});

		if (!mat->GetShaderReflection()->resources.empty())
		{
			auto it = mat->GetShaderReflection()->resources.begin();

			QLabel* setLabel = new QLabel(QString::fromStdString("Set " + std::to_string(it->set)), this);
			setLabel->setAlignment(Qt::AlignCenter);
			setLabel->setStyleSheet("font-weight: bold;");
			buffers->AddWidget(setLabel);

			while (it != mat->GetShaderReflection()->resources.end())
			{
				buffers->AddWidget(cp::SlangCompiler::CreateResourceWidget(*it, this));

				auto prev = it;
				it++;

				if (it != mat->GetShaderReflection()->resources.end() && it->set != prev->set)
				{
					setLabel = new QLabel(QString::fromStdString("Set " + std::to_string(it->set)), this);
					setLabel->setAlignment(Qt::AlignCenter);
					setLabel->setStyleSheet("font-weight: bold;");
					buffers->AddWidget(setLabel);
				}
			}
		}
#pragma endregion

		QPushButton* saveButton = new QPushButton("Save", this);
		layout->addWidget(saveButton);

		//+DEBUG

		QPushButton* compileButton = new QPushButton("Recompile", this);
		layout->addWidget(compileButton);

		connect(compileButton, &QPushButton::clicked, [=] {
			cp::SlangCompiler compiler;

			if (!mat)
			{
				LOG_ERROR("Material is null");
				return;
			}
			
			if (compiler.CompileMaterialSlangToSpirV(*mat))
			{
				LOG_INFO("Compiled shader to SPIR-V");
			}
			else
			{
				LOG_ERROR("Failed to compile shader to SPIR-V");
			}
			});

		//-DEBUG

		connect(saveButton, &QPushButton::clicked, [=] {
			cp::JsonSerializer serializer;
			mat->Serialize(serializer);
			serializer.Write(_path);
			});
	}

	void ShowMaterialInstance(const std::string& _path, const QFileInfo& _fileInfo)
	{
		readFile = new cp::MaterialInstance(scene->GetRenderer()->GetContext());
		cp::MaterialInstance* matInstance = static_cast<cp::MaterialInstance*>(readFile);
		cp::JsonSerializer serializer;
		serializer.Read(_path);
		matInstance->Deserialize(serializer);

		LOG_DEBUG(MF("Deserialized material instance from: ", _path));

		QWidget* matInstanceWidget = matInstance->CreateMaterialInstanceWidget(nullptr);

		LOG_DEBUG(MF("Created material instance widget for: ", _path));

		if (!matInstanceWidget)
		{
			LOG_ERROR("Failed to create material instance widget");
			return;
		}

		layout->addWidget(matInstanceWidget);

		QPushButton* saveButton = new QPushButton("Save", this);
		layout->addWidget(saveButton);
		connect(saveButton, &QPushButton::clicked, [=] {
			cp::JsonSerializer serializer;
			matInstance->Serialize(serializer);
			serializer.Write(_path);
			});
	}

	QLineEdit* CreateEntryPointOverride(cp::Material& _material, cp::ShaderStages _stage, const std::string& _placeholder, const std::string& _renderPass)
	{
		if (!_material.HasShaderStage(_stage)) return nullptr;

		QLineEdit* entryPoint = new QLineEdit(this);
		entryPoint->setPlaceholderText(QString::fromStdString(_placeholder));

		if(_material.GetRenderPassRequirement(_renderPass).customEntryPoints.find(_stage) != _material.GetRenderPassRequirement(_renderPass).customEntryPoints.end())
			entryPoint->setText(QString::fromStdString(_material.GetRenderPassRequirement(_renderPass).customEntryPoints.at(_stage)));

		return entryPoint;
	}

	QComboBox* CreateEntryPointOverrideComboBox(cp::Material& _material, cp::ShaderStages _stage, const std::string& _placeholder, const std::string& _renderPass)
	{
		if (!_material.HasShaderStage(_stage)) return nullptr;

		QComboBox* entryPoint = new QComboBox(this);
		entryPoint->setPlaceholderText(QString::fromStdString(_placeholder));

		if (_material.GetRenderPassRequirement(_renderPass).customEntryPoints.find(_stage) != _material.GetRenderPassRequirement(_renderPass).customEntryPoints.end())
			entryPoint->setCurrentText(QString::fromStdString(_material.GetRenderPassRequirement(_renderPass).customEntryPoints.at(_stage)));

		for (auto& [name, ep] : _material.GetShaderReflection()->entryPoints)
		{
			if (ep == _stage)
				entryPoint->addItem(QString::fromStdString(name));
		}

		if (entryPoint->currentText().isEmpty())
			entryPoint->setCurrentText(entryPoint->itemText(0));

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
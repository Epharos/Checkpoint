#include "Inspector.hpp"
#include "Inspector.hpp"
#include "Inspector.hpp"
#include "Inspector.hpp"
#include "../../Components/ComponentView.hpp"
#include "../../Components/Transform.hpp"
#include "../../CheckpointEditor.hpp"
#include "PrimitiveFields.hpp"
#include "FileDropPreviewWidget.hpp"

import EditorUI;

cp::Inspector::Inspector(QWidget* _parent)
{
	QWidget::QWidget(_parent);
	layout = new QVBoxLayout(this);
	this->setLayout(layout);
	titleLabel = new QLabel("Select an entity", this);
	layout->addWidget(titleLabel);
	layout->setAlignment(Qt::AlignTop);

	fileInspector["mat"] = [=](const std::string& _path) { ShowMaterial(_path); };
	fileInspector["matinstance"] = [=](const std::string& _path) { ShowMaterialInstance(_path); };
}

void cp::Inspector::Clear()
{
	QLayoutItem* item;

	while ((item = layout->takeAt(1)) != nullptr) {
		if (item->widget()) {
			delete item->widget();
		}

		delete item;
	}

	titleLabel->setText("Select an entity");
}

void cp::Inspector::ShowEntity(cp::EntityAsset* _entity)
{
	Clear();

	if (!_entity) {
		return;
	}

	titleLabel->setText(QString::fromStdString(_entity->name.empty() ? "Entity" : _entity->name));

	QFrame* line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	line->setStyleSheet("background-color: #3E465A; margin-top: 4px; margin-bottom: 4px; min-height: 1px; max-height: 1px;");
	layout->addWidget(line);

	UpdateEntityComponents(_entity);

	layout->addStretch();
}

void cp::Inspector::ShowFile(const std::string& _path)
{
	Clear();

	titleLabel->setText(QString::fromStdString("File: " + cp::CheckpointEditor::CurrentProject.GetResourceRelativePath(_path)));

	if (readFile) {
		delete readFile;
		readFile = nullptr;
	}

	std::string extension = _path.substr(_path.find_last_of('.') + 1);
	if (fileInspector.find(extension) != fileInspector.end()) {
		fileInspector[extension](_path);
	} else {
		QLabel* label = new QLabel("No inspector available for this file type", this);
		layout->addWidget(label);
		layout->addStretch();
	}
}

void cp::Inspector::ShowMaterial(const std::string& _path) {
	readFile = new cp::Material(&cp::CheckpointEditor::VulkanCtx);
	cp::Material* mat = static_cast<cp::Material*>(readFile);
	cp::JsonSerializer serializer;
	serializer.Read(_path);
	mat->Deserialize(serializer);

	QVBoxLayout* matLayout = new QVBoxLayout();
	matLayout->setContentsMargins(0, 0, 0, 0);
	matLayout->setSpacing(8);
	matLayout->setAlignment(Qt::AlignTop);
	matLayout->setSizeConstraint(QLayout::SetFixedSize);

#pragma region Material Properties
	cp::StringField* materialNameField = new cp::StringField(mat->GetNamePtr(), "Name");
	matLayout->addWidget(materialNameField);

	cp::FileDropPreviewWidget* shaderPathField = new cp::FileDropPreviewWidget(
		mat->GetShaderPathPtr(),
		"Shader Path",
		{ "slang" },
		QString::fromStdString(cp::CheckpointEditor::CurrentProject.GetResourcePath())
	);

	matLayout->addWidget(shaderPathField);
#pragma endregion

	QFrame* line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	line->setStyleSheet("background-color: #3E465A; margin-top: 4px; margin-bottom: 4px; min-height: 1px; max-height: 1px;");
	matLayout->addWidget(line);

	cp::QtEditorUIFactory factory;

#pragma region Debug Collapse

	{
		auto collapsible = factory.CreateCollapsible().release();
		collapsible->SetTitle("Debug");
		auto container = factory.CreateContainer().release();
		QWidget* containerWidget = static_cast<QWidget*>(container->NativeHandle());

		std::set<cp::ShaderStages> uniqueShaderStages;

		for (const auto& [_, stage] : mat->GetShaderReflection()->entryPoints)
		{
			uniqueShaderStages.insert(stage);
		}

		if (uniqueShaderStages.size() >= 1)
		{
			
			auto entryPointsLabel = factory.CreateLabel("Shader Stages");
			container->AddChild(entryPointsLabel.release());

			for (const auto& stage : uniqueShaderStages)
			{
				auto shaderStageLabel = factory.CreateLabel(Helper::Material::GetShaderStageString(stage));
				shaderStageLabel->SetColor(cp::HexColorToUInt32("#aaaaaa"));
				container->AddChild(shaderStageLabel.release());
			}
		}

		collapsible->SetContent(container);
		matLayout->addWidget(static_cast<QWidget*>(collapsible->NativeHandle()));
	}

#pragma endregion

	line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	line->setStyleSheet("background-color: #3E465A; margin-top: 4px; margin-bottom: 4px; min-height: 1px; max-height: 1px;");
	matLayout->addWidget(line);

#pragma region Renderpasses

	{
		auto collapsible = factory.CreateCollapsible().release();
		collapsible->SetTitle("Render passes");
		auto container = factory.CreateContainer().release();

		auto CreateEntryPointOverrideCombobox = [&](cp::Material& _mat, const cp::ShaderStages& _stage, const std::string& _placeholder, const std::string& _renderpass) -> cp::IComboBox*
		{
			if (!_mat.HasShaderStage(_stage)) return nullptr;

			auto entryPoints = factory.CreateComboBox();
			entryPoints->SetPlaceholderText(_placeholder);

			for (auto& [name, ep] : _mat.GetShaderReflection()->entryPoints)
			{
				if (ep == _stage) entryPoints->AddItem(name);
			}

			if (_mat.GetRenderPassRequirement(_renderpass).customEntryPoints.contains(_stage))
			{
				entryPoints->SetSelectedItem(_mat.GetRenderPassRequirement(_renderpass).customEntryPoints.at(_stage));
			}

			if (entryPoints->GetSelectedItem().empty())
			{
				entryPoints->SetSelectedIndex(0);
				_mat.GetRenderPassRequirement(_renderpass).customEntryPoints[_stage] = entryPoints->GetSelectedItem();
			}

			entryPoints->SetOnSelectionChangedListener([=, &_mat](const std::string& selectedItem) {
				_mat.GetRenderPassRequirement(_renderpass).customEntryPoints[_stage] = selectedItem;
			});

			return entryPoints.release();
		};

		for (const auto& [rpName, rpDesc] : cp::CheckpointEditor::CurrentScene->renderer->GetRenderPassDescriptions())
		{
			auto& rpReq = mat->GetRenderPassRequirement(rpName);

			auto renderActive = factory.CreateCheckBox(rpName).release();
			renderActive->SetChecked(rpReq.renderToPass);

			auto useDefaultShader = factory.CreateCheckBox("Use default shader").release();
			useDefaultShader->SetChecked(rpReq.useDefaultShader);
			useDefaultShader->SetVisible(renderActive->IsChecked() && rpDesc.GetDefaultPipeline().has_value());

			auto entryPointCollapsible = factory.CreateCollapsible().release();
			entryPointCollapsible->SetTitle("Entry Points");
			auto entryPointsContainer = factory.CreateContainer().release();

			if (auto ep = CreateEntryPointOverrideCombobox(*mat, cp::ShaderStages::Vertex, "Vertex_Default", rpName))
			{
				entryPointsContainer->AddChild(ep);
			}

			if (!rpDesc.IsDepthOnly())
			{
				if (auto ep = CreateEntryPointOverrideCombobox(*mat, cp::ShaderStages::Fragment, "Fragment_Default", rpName))
				{
					entryPointsContainer->AddChild(ep);
				}
			}

			if (auto ep = CreateEntryPointOverrideCombobox(*mat, cp::ShaderStages::Geometry, "Geometry_Default", rpName))
			{
				entryPointsContainer->AddChild(ep);
			}

			if (auto ep = CreateEntryPointOverrideCombobox(*mat, cp::ShaderStages::TessellationControl, "TessellationControl_Default", rpName))
			{
				entryPointsContainer->AddChild(ep);
			}

			if (auto ep = CreateEntryPointOverrideCombobox(*mat, cp::ShaderStages::TessellationEvaluation, "TessellationEvaluation_Default", rpName))
			{
				entryPointsContainer->AddChild(ep);
			}

			if (auto ep = CreateEntryPointOverrideCombobox(*mat, cp::ShaderStages::Mesh, "Mesh_Default", rpName))
			{
				entryPointsContainer->AddChild(ep);
			}

			entryPointCollapsible->SetContent(entryPointsContainer);
			entryPointCollapsible->SetVisible(renderActive->IsChecked() && (!useDefaultShader->IsChecked() || !rpDesc.GetDefaultPipeline().has_value()));

			renderActive->SetOnCheckedChangedListener([=, &rpReq](bool checked) {
				rpReq.renderToPass = checked;
				useDefaultShader->SetVisible(checked && rpDesc.GetDefaultPipeline().has_value());
				entryPointCollapsible->SetVisible(checked && (!useDefaultShader->IsChecked() || !rpDesc.GetDefaultPipeline().has_value()));
				});

			useDefaultShader->SetOnCheckedChangedListener([=, &rpReq](bool checked) {
				rpReq.useDefaultShader = checked;
				entryPointCollapsible->SetVisible(renderActive->IsChecked() && !checked);
				});

			container->AddChild(renderActive);
			container->AddChild(useDefaultShader);
			container->AddChild(entryPointCollapsible);

			auto separator = factory.CreateHorizontalSeparator().release();
			container->AddChild(separator);
		}

		collapsible->SetContent(container);
		matLayout->addWidget(static_cast<QWidget*>(collapsible->NativeHandle()));
	}

#pragma endregion

#pragma region Buffers

	{
		auto collapsible = factory.CreateCollapsible().release();
		collapsible->SetTitle("Buffers");
		auto container = factory.CreateContainer().release();

		std::sort(mat->GetShaderReflection()->resources.begin(), mat->GetShaderReflection()->resources.end(), [](const cp::ShaderResource& a, const cp::ShaderResource& b) {
			return a.set < b.set && a.binding < b.binding;
		});

		if (!mat->GetShaderReflection()->resources.empty())
		{
			auto it = mat->GetShaderReflection()->resources.begin();

			auto setLabel = factory.CreateLabel("Set " + std::to_string(it->set));
			setLabel->SetBold(true);
			container->AddChild(setLabel.release());

			while (it != mat->GetShaderReflection()->resources.end())
			{
				QWidget* resourceWidget = reinterpret_cast<QWidget*>(container->NativeHandle());

				if (!resourceWidget) LOG_ERROR("Failed to get native handle for resource widget");
				if (!resourceWidget->layout()) LOG_ERROR("Resource widget has no layout");

				QWidget* newWidget = cp::SlangCompiler::CreateResourceWidget(*it, nullptr);

				if(newWidget) resourceWidget->layout()->addWidget(newWidget); 
				// TODO: Use Factory
				// TODO: SlangCompiler should be an editor-only class

				auto prev = it;
				it++;

				if (it != mat->GetShaderReflection()->resources.end() && it->set != prev->set)
				{
					setLabel = factory.CreateLabel("Set " + std::to_string(it->set));
					setLabel->SetBold(true);
					container->AddChild(setLabel.release());
				}
			}
		}

		collapsible->SetContent(container);
		matLayout->addWidget(static_cast<QWidget*>(collapsible->NativeHandle()));
	}

#pragma endregion

#pragma region Bottom Buttons

	{
		auto container = factory.CreateContainer().release();
		container->SetHorizontal();
		container->SetSpacing(8);

		auto saveButton = factory.CreateFlatButton("Save Material").release();
		saveButton->SetOnClickListener([=]() {
			cp::JsonSerializer serializer;
			mat->Serialize(serializer);
			serializer.Write(_path);
			});

		auto recompileButton = factory.CreateFlatButton("Recompile Shader").release();
		recompileButton->SetOnClickListener([=]() {
			cp::SlangCompiler compiler;

			if(!mat) {
				LOG_ERROR("No material loaded to recompile shader for");
				return;
			}

			if (compiler.CompileMaterialSlangToSpirV(*mat)) {
				LOG_INFO("Shader recompiled successfully");
			} else {
				LOG_ERROR("Failed to recompile shader");
			}
		});

		container->AddChild(saveButton);
		container->AddChild(recompileButton);
		matLayout->addWidget(static_cast<QWidget*>(container->NativeHandle()));
	}

#pragma endregion

	QWidget* matWidget = new QWidget(this);
	matWidget->setLayout(matLayout);
	matWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	matLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
	layout->setSizeConstraint(QLayout::SetDefaultConstraint);

	layout->addWidget(matWidget);

	layout->addStretch();
}

void cp::Inspector::ShowMaterialInstance(const std::string& _path)
{
	readFile = new cp::MaterialInstance(&cp::CheckpointEditor::VulkanCtx);
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

	QPushButton* saveButton = new QPushButton("Save Material Instance", this);
	layout->addWidget(saveButton);
	connect(saveButton, &QPushButton::clicked, [=] {
		cp::JsonSerializer serializer;
		matInstance->Serialize(serializer);
		serializer.Write(_path);
		});
}

void cp::Inspector::UpdateEntityComponents(cp::EntityAsset* _entity)
{
	cp::QtEditorUIFactory factory;

	QLayoutItem* child;
	while ((child = layout->takeAt(2)) != nullptr)
	{
		if (child->widget()) child->widget()->deleteLater();
		else if (child->layout()) child->layout()->deleteLater();
		else delete child;
	}

	for (size_t i = 0; i < _entity->GetComponents().size(); ++i) {
		auto view = cp::ComponentViewRegistry::GetInstance().CreateView(_entity->GetComponents()[i]);
		auto collapsible = factory.CreateCollapsible().release();
		collapsible->SetTitle(view->GetName());
		collapsible->SetContent(view->Render(&factory));
		layout->addWidget(static_cast<QWidget*>(collapsible->NativeHandle()));

		if (i != _entity->GetComponents().size() - 1) {
			QFrame* line = new QFrame(this);
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			line->setStyleSheet("background-color: #3E465A; margin-top: 4px; margin-bottom: 4px; min-height: 1px; max-height: 1px;");
			layout->addWidget(line);
		}
	}

	CreateAddComponentButton(_entity);
}

void cp::Inspector::CreateAddComponentButton(cp::EntityAsset* _entity)
{
	//TODO : Rework this to use the EditorUIFactory

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
			IComponentBase* component = reinterpret_cast<IComponentBase*>(cp::ComponentRegistry::GetInstance().CreateComponentInstance(_type));
			_entity->AddComponent(component);

			UpdateEntityComponents(_entity);

			if (menu) menu->close();
		});

		menu->popup(addComponentButton->mapToGlobal(QPoint(0, addComponentButton->height())));
		});
}

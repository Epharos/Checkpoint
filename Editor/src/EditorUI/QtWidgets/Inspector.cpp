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

	cp::QtEditorUIFactory factory;

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

	layout->addStretch(1);
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
		layout->addStretch(1);
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
			QLabel* entryPointsLabel = new QLabel("Shader stages", this);
			containerWidget->layout()->addWidget(entryPointsLabel);

			for (const auto& stage : uniqueShaderStages)
			{
				QLabel* shaderStage = new QLabel(this);
				shaderStage->setText(QString::fromStdString(Helper::Material::GetShaderStageString(stage)));
				shaderStage->setStyleSheet("color : #aaa;");
				containerWidget->layout()->addWidget(shaderStage);
			}
		}

		collapsible->SetContent(container);
		matLayout->addWidget(static_cast<QWidget*>(collapsible->NativeHandle()));
	}

#pragma endregion




	QWidget* matWidget = new QWidget(this);
	matWidget->setLayout(matLayout);
	matWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

	layout->addWidget(matWidget);

	layout->addStretch(1);
}
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

		Collapsible* renderpasses = new Collapsible("Render passes", this);
		layout->addWidget(renderpasses);

		for (auto& rp : scene->GetRenderer()->GetRenderPassNames())
		{
			QLabel* rpLabel = new QLabel(QString::fromStdString(rp), this);
			renderpasses->AddWidget(rpLabel);

			QCheckBox* rpCheckBox = new QCheckBox("Require unique shader", this);
			renderpasses->AddWidget(rpCheckBox);

			rpCheckBox->setChecked(mat->GetRenderPassRequirement(rp).requireUniqueShader);

			connect(rpCheckBox, &QCheckBox::checkStateChanged, [=](Qt::CheckState _state) {
				mat->GetRenderPassRequirement(rp).requireUniqueShader = _state == Qt::CheckState::Checked;
				});

			renderpasses->AddSpacing(10);
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
};
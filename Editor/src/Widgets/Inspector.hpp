#pragma once

#include "../pch.hpp"
#include "SearchList.hpp"

#include "Widgets/ComponentFields/String.hpp"
#include "Widgets/ComponentFields/FileDropLineEdit.hpp"

class Inspector : public QWidget
{
	Q_OBJECT

protected:
	QVBoxLayout* layout;
	QLabel* titleLabel;
	cp::Scene* scene = nullptr;

	void* readFile = nullptr;

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

		if (_fileInfo.suffix().endsWith("mat")) // Material file
		{
			readFile = new cp::Material(scene->GetRenderer()->GetContext());
			cp::Material* tmp = static_cast<cp::Material*>(readFile);
			cp::JsonSerializer serializer;
			serializer.Read(_path);
			tmp->Deserialize(serializer);
			String* nameMat = new String(tmp->GetNamePtr(), "Material name", this);
			layout->addWidget(nameMat);
			layout->addSpacing(20);

			FileDropLineEdit* shaderPath = new FileDropLineEdit(this);
			shaderPath->SetResourcePath(tmp->GetShaderPath());
			shaderPath->SetAcceptedExtensions({ "slang" });
			layout->addWidget(shaderPath);

			QPushButton* saveButton = new QPushButton("Save", this);
			layout->addWidget(saveButton);

			connect(shaderPath, &FileDropLineEdit::ResourcePathChanged, [=](const std::string& _path) {
				tmp->SetShaderPath(_path);
				});

			connect(saveButton, &QPushButton::clicked, [=] {
				cp::JsonSerializer serializer;
				tmp->Serialize(serializer);
				serializer.Write(_path);
				});
		}

		layout->update();
	}
};
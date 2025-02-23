#pragma once

#include "../pch.hpp"
#include "SearchList.hpp"

class Inspector : public QWidget
{
	Q_OBJECT

protected:
	QVBoxLayout* layout;
	QLabel* titleLabel;
	Core::Scene* scene = nullptr;

public:
	Inspector(Core::Scene* _scene, QWidget* _parent = nullptr) : QWidget(_parent), scene(_scene)
	{
		layout = new QVBoxLayout(this);
		titleLabel = new QLabel("Select an entity or a file", this);
		layout->addWidget(titleLabel);
		setLayout(layout);
		layout->setAlignment(Qt::AlignTop);


		setMinimumWidth(240);
		setMinimumHeight(480);
	}

	void CreateAddComponentButton(Entity* _entity)
	{
		QPushButton* addComponentButton = new QPushButton("Add Component", this);
		layout->addWidget(addComponentButton);

		connect(addComponentButton, &QPushButton::clicked, [=] {
			QMenu* menu = new QMenu(addComponentButton);

			SearchList* searchList = new SearchList(menu);
			searchList->Populate(ComponentRegistry::GetInstance().GetTypeIndexMap());

			QWidgetAction* widgetAction = new QWidgetAction(menu);
			widgetAction->setDefaultWidget(searchList);
			menu->addAction(widgetAction);

			connect(searchList, &SearchList::ItemSelected, [=](std::type_index _type) {
				ComponentRegistry::GetInstance().CreateComponent(scene->GetECS(), *_entity, _type);
				UpdateComponents(_entity);
				if (menu) menu->close();
				});

			menu->popup(addComponentButton->mapToGlobal(QPoint(0, addComponentButton->height())));
			});
	}

	void UpdateComponents(Entity* _entity)
	{
		QLayoutItem* child;
		while ((child = layout->takeAt(2)) != nullptr)
		{
			if(child->widget()) child->widget()->deleteLater();
			else delete child;
		}

		for (auto& [type, component] : scene->GetECS().GetAllComponentsOf(*_entity))
		{
			auto widget = ComponentRegistry::GetInstance().CreateWidget(scene->GetECS(), *_entity, type);
			widget->Initialize();
			layout->addWidget(widget.release());
			layout->addSpacerItem(new QSpacerItem(0, 10));
		}

		CreateAddComponentButton(_entity);

		layout->update();
	}

	void ShowEntity(Entity* _entity)
	{
		titleLabel->setText(QString::fromStdString(_entity->GetDisplayName()));

		layout->addSpacerItem(new QSpacerItem(0, 10));

		UpdateComponents(_entity);
	}

	void ShowFile(const std::string& _path)
	{
		size_t lastSlash = _path.find_last_of("\\");
		size_t lastDot = _path.find_last_of(".");

		QString fileName = QString::fromStdString(_path.substr(lastSlash + 1, lastDot - lastSlash - 1));

		titleLabel->setText(fileName);

		QLayoutItem* child;
		while ((child = layout->takeAt(1)) != nullptr)
		{
			delete child->widget();
			delete child;
		}
	}
};
#pragma once

#include "../pch.hpp"

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
	}

	void ShowEntity(Entity* _entity)
	{
		titleLabel->setText(QString::fromStdString(_entity->GetDisplayName()));

		QLayoutItem* child;
		while ((child = layout->takeAt(1)) != nullptr)
		{
			delete child->widget();
			delete child;
		}

		/*for (auto& component : scene->GetECS().GetEntityComponents(_entity))
		{
			QLabel* componentLabel = new QLabel(QString::fromStdString(component->GetDisplayName()), this);
			layout->addWidget(componentLabel);
		}*/
	}

	void ShowFile(const std::string& _path)
	{
		titleLabel->setText(QString::fromStdString(_path));

		QLayoutItem* child;
		while ((child = layout->takeAt(1)) != nullptr)
		{
			delete child->widget();
			delete child;
		}
	}
};
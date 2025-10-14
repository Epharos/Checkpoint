#pragma once

#include "../../pch.hpp"
#include "Button.hpp"

class ProjectOverview : public QWidget
{
	Q_OBJECT

protected:
	QVBoxLayout* layout;
public:
	ProjectOverview(const ProjectData& _data, QWidget* parent = nullptr) : QWidget(parent)
	{
		layout = new QVBoxLayout(this);
		
		SetProject(_data);

		setFixedWidth(340);
	}

	ProjectOverview(QWidget* parent = nullptr) : QWidget(parent)
	{
		QLabel* label = new QLabel("Select a project", this);
		label->setStyleSheet("font-size: 24px; font-weight: 600;");

		layout = new QVBoxLayout(this);
		layout->setAlignment(Qt::AlignCenter);
		layout->addWidget(label);

		setFixedWidth(340);
	}

	void SetProject(const ProjectData& _data)
	{
		QLayoutItem* child;
		while ((child = layout->takeAt(0)) != nullptr)
		{
			if (child->widget())
				child->widget()->deleteLater();
			else if (child->layout())
			{
				QLayoutItem* tmp;
				while ((tmp = child->layout()->takeAt(0)) != nullptr)
				{
					if (tmp->widget())
						tmp->widget()->deleteLater();
					else
						delete tmp;
				}

				child->layout()->deleteLater();
			}
			else 
				delete child;
		}

		layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

		QLabel* label = new QLabel(_data.name.toUpper(), this);
		label->setStyleSheet("font-size: 24px; font-weight: 600;");

		QHBoxLayout* createdLayout = new QHBoxLayout();
		createdLayout->setAlignment(Qt::AlignLeft);
		QLabel* createdLabel = new QLabel("Created: ", this);
		QLabel* createdValue = new QLabel(_data.creationDate.toString("dd.MM.yyyy HH:mm"), this);
		createdValue->setStyleSheet("font-weight: 600;");
		createdLayout->addWidget(createdLabel);
		createdLayout->addWidget(createdValue);

		QHBoxLayout* lastOpenedLayout = new QHBoxLayout();
		lastOpenedLayout->setAlignment(Qt::AlignLeft);
		QLabel* lastOpenedLabel = new QLabel("Last Opened: ", this);
		QLabel* lastOpenedValue = new QLabel(_data.lastOpened.toString("dd.MM.yyyy HH:mm"), this);
		lastOpenedValue->setStyleSheet("font-weight: 600;");
		lastOpenedLayout->addWidget(lastOpenedLabel);
		lastOpenedLayout->addWidget(lastOpenedValue);

		QHBoxLayout* engineVersionLayout = new QHBoxLayout();
		engineVersionLayout->setAlignment(Qt::AlignLeft);
		QLabel* engineVersionLabel = new QLabel("Engine Version: ", this);
		QLabel* engineVersionValue = new QLabel(QString::fromStdString(cp::VulkanContext::VersionToString(_data.engineVersion)), this);
		engineVersionValue->setStyleSheet("font-weight: 600;");
		engineVersionLayout->addWidget(engineVersionLabel);
		engineVersionLayout->addWidget(engineVersionValue);

		QHBoxLayout* graphicEngineLayout = new QHBoxLayout();
		graphicEngineLayout->setAlignment(Qt::AlignLeft);
		QLabel* graphicEngineLabel = new QLabel("Graphic Engine: ", this);
		QLabel* graphicEngineValue = new QLabel("Vulkan", this);
		graphicEngineValue->setStyleSheet("font-weight: 600;");
		graphicEngineLayout->addWidget(graphicEngineLabel);
		graphicEngineLayout->addWidget(graphicEngineValue);

		layout->addWidget(label);
		layout->addSpacing(10);
		layout->addLayout(createdLayout);
		layout->addLayout(lastOpenedLayout);
		layout->addSpacing(10);
		layout->addLayout(engineVersionLayout);
		layout->addLayout(graphicEngineLayout);

		QHBoxLayout* buttonLayout = new QHBoxLayout();
		buttonLayout->setAlignment(Qt::AlignRight);
		Button* openProjectBtn = new Button("Open", ButtonImportance::Primary, this);
		Button* deleteProjectBtn = new Button("Delete", ButtonImportance::Secondary, this);
		deleteProjectBtn->SetCustomColor("#dc3545");
		buttonLayout->addWidget(deleteProjectBtn);
		buttonLayout->addWidget(openProjectBtn);

		layout->addStretch();
		layout->addLayout(buttonLayout);

		layout->update();
	}

signals:
	void OpenProject(const std::string& _path);
	void DeleteProject(const std::string& _path);

private slots:
	void OpenProject()
	{
		emit OpenProject("path");
	}

	void DeleteProject()
	{
		emit DeleteProject("path");
	}
};
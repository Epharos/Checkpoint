#pragma once

#include "pch.hpp"

#include "NewProjectDialog.hpp"
#include "MainWindow.hpp"

class ProjectListItemWidget : public QWidget
{
    Q_OBJECT

protected:
	ProjectData data;
    QList<ProjectData>* list;

    void DeleteItemFromList()
    {

    }

public:
	ProjectListItemWidget(const ProjectData& data, QWidget* parent = nullptr) : QWidget(parent), data(data), list(nullptr)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);

		QLabel* nameLabel = new QLabel(data.name, this);
		nameLabel->setStyleSheet("font-weight: 600; font-size:16px");
		QLabel* pathLabel = new QLabel(data.path, this);
		pathLabel->setStyleSheet("font-size:12px; color:#888;");
		QLabel* lastOpenedLabel = new QLabel(data.lastOpened.toString("yyyy-MM-dd HH:mm"), this);
		lastOpenedLabel->setStyleSheet("font-size:12px; color:#bbb;");

        QPushButton* deleteBtn = new QPushButton("Delete", this);
        deleteBtn->setStyleSheet("background-color:#f33; border-color:#ddd; border-radius:5px; font-size:10px;");
        deleteBtn->setFixedSize(60, 16);

		layout->addWidget(nameLabel);
        layout->addWidget(lastOpenedLabel);
		layout->addWidget(pathLabel);
        layout->addWidget(deleteBtn);

        connect(deleteBtn, &QPushButton::clicked, this, &ProjectListItemWidget::DeleteItemFromList);
	}

	const ProjectData& GetData() const { return data; }
};

class Launcher : public QWidget {
    Q_OBJECT

public:
    Launcher() {
        setWindowTitle("Checkpoint Launcher");
        //setFixedSize(400, 200);

        QVBoxLayout* layout = new QVBoxLayout(this);
		QLabel* titleLabel = new QLabel("Recent Projects", this);
		titleLabel->setStyleSheet("font-size:24px; font-weight:bold;");
        listWidget = new QListWidget(this);
        listWidget->setMinimumSize(500, 350);
        QPushButton* newProjectBtn = new QPushButton("New Project", this);
		newProjectBtn->setFixedSize(120, 30);
        newProjectBtn->setStyleSheet("font-size:16px; color:#fff; background-color:#007bff; border:none; padding:4px 20px; border-radius:5px;");
		QPushButton* openProjectBtn = new QPushButton("Open Project", this);
		openProjectBtn->setFixedSize(120, 30);
		openProjectBtn->setStyleSheet("font-size:16px; color:#fff; background-color:#555; border:none; padding:4px 20px; border-radius:5px;");
		QHBoxLayout* buttonLayout = new QHBoxLayout(this);
        buttonLayout->addWidget(openProjectBtn);
		buttonLayout->addWidget(newProjectBtn);
		buttonLayout->setAlignment(Qt::AlignRight);

        layout->addWidget(titleLabel);
		layout->addLayout(buttonLayout);
        layout->addWidget(listWidget);

        QList<ProjectData> projects = LoadRecentProjects();

        for (const auto& project : projects)
        {
            ProjectListItemWidget* item = new ProjectListItemWidget(project);

            QListWidgetItem* listItem = new QListWidgetItem();
            listItem->setSizeHint(item->sizeHint());
            listWidget->addItem(listItem);
            listWidget->setItemWidget(listItem, item);
        }

        connect(listWidget, &QListWidget::itemDoubleClicked, this, &Launcher::OpenProject);
        connect(newProjectBtn, &QPushButton::clicked, this, &Launcher::CreateNewProject);
    }
    
private slots:
    void OpenProject(QListWidgetItem* item) 
    {
		ProjectListItemWidget* widget = dynamic_cast<ProjectListItemWidget*>(listWidget->itemWidget(item));

		if (widget)
		{
			ProjectData data = widget->GetData();
			MainWindow* mainWindow = new MainWindow(data);
			mainWindow->setWindowTitle("Checkpoint - " + data.name);
			mainWindow->show();
		}

        this->close();
    }

    void CreateNewProject() 
    {
        NewProjectDialog* dialog = new NewProjectDialog(this);

        connect(dialog, &NewProjectDialog::ProjectCreated, this, &Launcher::SaveNewProject);

        dialog->exec();
    }

    void SaveNewProject(const ProjectData& projectData) 
    {
        QList<ProjectData> projects = LoadRecentProjects();

        if (!projects.contains(projectData)) 
        {
            projects.prepend(projectData);
            SaveRecentProjects(projects);

            listWidget->clear();
            
			for (const auto& project : projects)
			{
				ProjectListItemWidget* item = new ProjectListItemWidget(project);

				QListWidgetItem* listItem = new QListWidgetItem();
				listItem->setSizeHint(item->sizeHint());
				listWidget->addItem(listItem);
				listWidget->setItemWidget(listItem, item);
			}
        }
    }

    void SaveRecentProjects(const QList<ProjectData>& projects) 
    {
        QJsonArray jsonArray;

        for (const auto& project : projects) 
        {
			QJsonObject obj;
			obj["name"] = project.name;
			obj["path"] = project.path;
			obj["lastOpened"] = project.lastOpened.toString("yyyy-MM-dd HH:mm");
            jsonArray.append(obj);
        }

        QJsonObject root;
        root["recentProjects"] = jsonArray;

        QFile file("recent_projects.json");
        if (file.open(QIODevice::WriteOnly)) 
        {
            file.write(QJsonDocument(root).toJson());
        }
    }

    QList<ProjectData> LoadRecentProjects() 
    {
        QFile file("recent_projects.json");
        if (!file.open(QIODevice::ReadOnly)) return {};

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray jsonArray = doc.object()["recentProjects"].toArray();

        QList<ProjectData> projects;

        for (const auto& value : jsonArray)
        {
            QJsonObject obj = value.toObject();
            projects.append(ProjectData(obj["name"].toString(), obj["path"].toString(), QDateTime::fromString(obj["lastOpened"].toString(), "yyyy-MM-dd HH:mm")));
        }

        return projects;
    }

private:
    QListWidget* listWidget;
};
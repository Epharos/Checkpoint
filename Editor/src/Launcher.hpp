#pragma once

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qdialog.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qfile.h>

#include "NewProjectDialog.hpp"

class ProjectListItemWidget : public QWidget
{
    Q_OBJECT

protected:
	ProjectData data;

public:
	ProjectListItemWidget(const ProjectData& data, QWidget* parent = nullptr) : QWidget(parent), data(data)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);

		QLabel* nameLabel = new QLabel(data.name, this);
		nameLabel->setStyleSheet("font-weight: bold; font-size:16px");
		QLabel* pathLabel = new QLabel(data.path, this);
		pathLabel->setStyleSheet("font-size:12px; color:#888;");
		QLabel* lastOpenedLabel = new QLabel(data.lastOpened.toString("yyyy-MM-dd HH:mm:ss"), this);
		lastOpenedLabel->setStyleSheet("font-size:12px; color:#bbb;");

		layout->addWidget(nameLabel);
        layout->addWidget(lastOpenedLabel);
		layout->addWidget(pathLabel);
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
        listWidget = new QListWidget(this);
        QPushButton* newProjectBtn = new QPushButton("New Project", this);
        newProjectBtn->setStyleSheet("font-size:12px; font-weight:bold; color:#fff; background-color:#007bff; border:none; padding:4px 20px; border-radius:5px;");
        listWidget->setMinimumSize(500, 350);

        layout->addWidget(newProjectBtn);
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

        connect(listWidget, &QListWidget::itemClicked, this, &Launcher::OpenProject);
        connect(newProjectBtn, &QPushButton::clicked, this, &Launcher::CreateNewProject);
    }
    
private slots:
    void OpenProject(QListWidgetItem* item) 
    {
        QString projectPath = item->text();
        qDebug() << "Opening project at:" << projectPath;
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
			obj["lastOpened"] = project.lastOpened.toString("yyyy-MM-dd HH:mm:ss");
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
            projects.append(ProjectData(obj["name"].toString(), obj["path"].toString(), QDateTime::fromString(obj["lastOpened"].toString(), "yyyy-MM-dd HH:mm:ss")));
        }

        return projects;
    }

private:
    QListWidget* listWidget;
};
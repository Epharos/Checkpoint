#pragma once

#include "pch.hpp"

#include "NewProjectDialog.hpp"
//#include "MainWindow.hpp"

#include "Widgets/Launcher/Button.hpp"
#include "Widgets/Launcher/ProjectTableView.hpp"
#include "Widgets/Launcher/ProjectOverview.hpp"

class Launcher : public QWidget
{
    Q_OBJECT

public:
    Launcher() {
        setWindowTitle("Checkpoint - Launcher");
		setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
        setWindowIcon(QIcon("Editor_Resources/template.png"));

		setFixedHeight(700);

        QVBoxLayout* layout = new QVBoxLayout(this);

		QLabel* titleLabel = new QLabel("Recent Projects", this);
		titleLabel->setStyleSheet("font-size:24px; font-weight:bold;");

		Button* createNewProjectBtn = new Button("Create", ButtonImportance::Primary, this);
		Button* openProjectBtn = new Button("Load", ButtonImportance::Secondary, this);
		QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(openProjectBtn);
		buttonLayout->addWidget(createNewProjectBtn);
		buttonLayout->setAlignment(Qt::AlignRight);

		tableView = new ProjectTableView(this);
		projectOverview = new ProjectOverview(this);

		QHBoxLayout* tableViewLayout = new QHBoxLayout();
		tableViewLayout->addWidget(tableView);
		tableViewLayout->addWidget(projectOverview);

        layout->addWidget(titleLabel);
		layout->addLayout(buttonLayout);
		layout->addLayout(tableViewLayout);

        std::vector<ProjectData> projects = LoadRecentProjects();

        tableView->Populate(projects);

        connect(createNewProjectBtn, &QPushButton::clicked, this, &Launcher::CreateNewProject);
		connect(tableView, &ProjectTableView::ProjectOpened, this, &Launcher::OpenProject);
        connect(tableView, &ProjectTableView::ItemSelected, this, &Launcher::ShowProjectData);
    }
    
private slots:
    void OpenProject(const QString& _path) 
    {
		QFile file(_path + "/project.data");
		if (!file.open(QIODevice::ReadWrite)) return;

		QJsonObject projectDataObject = QJsonDocument::fromJson(file.readAll()).object();
		ProjectData projectData = ProjectData::FromJson(projectDataObject);
		projectData.lastOpened = QDateTime::currentDateTime();
        
		projectDataObject = ProjectData::ToJson(projectData);

		file.seek(0);
		file.write(QJsonDocument(projectDataObject).toJson());

		//MainWindow* mainWindow = new MainWindow(projectData);
		//mainWindow->setWindowTitle(projectData.name + " - Checkpoint");
		//mainWindow->show();

        close();
    }

	void ShowProjectData(const QString& _path)
	{
		QFile file(_path + "/project.data");
		if (!file.open(QIODevice::ReadOnly)) return;

		QJsonObject projectDataObject = QJsonDocument::fromJson(file.readAll()).object();
		ProjectData projectData = ProjectData::FromJson(projectDataObject);

		projectOverview->SetProject(projectData);
	}

    void CreateNewProject() 
    {
        NewProjectDialog* dialog = new NewProjectDialog(this);

        connect(dialog, &NewProjectDialog::ProjectCreated, this, &Launcher::SaveNewProject);

        dialog->exec();
    }

    void SaveNewProject(const ProjectData& projectData) 
    {
        std::vector<ProjectData> projects = LoadRecentProjects();

		auto it = std::find(projects.begin(), projects.end(), projectData);
        if (it != projects.end())
            return;

		projects.insert(projects.begin(), projectData);
        SaveRecentProjects(projects);

		tableView->Populate(projects);
    }

    void SaveRecentProjects(const std::vector<ProjectData>& projects) 
    {
        QJsonArray jsonArray;

        for (const auto& project : projects) 
        {
            jsonArray.append(project.path);
        }

        QJsonObject root;
        root["recentProjects"] = jsonArray;

        QFile file("recent_projects.json");
        if (file.open(QIODevice::WriteOnly)) 
        {
            file.write(QJsonDocument(root).toJson());
        }
    }

    std::vector<ProjectData> LoadRecentProjects() 
    {
        QFile file("recent_projects.json");
        if (!file.open(QIODevice::ReadOnly)) return {};

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray jsonArray = doc.object()["recentProjects"].toArray();

        std::vector<ProjectData> projects;

        for (const auto& value : jsonArray)
        {
			QFile projectDataFile(value.toString() + "/project.data");
			if (!projectDataFile.open(QIODevice::ReadOnly)) continue;

			QJsonDocument projectDoc = QJsonDocument::fromJson(projectDataFile.readAll());
			QJsonObject projectObj = projectDoc.object();

			projects.push_back(ProjectData::FromJson(projectObj));
        }

		std::sort(projects.begin(), projects.end(), [](const ProjectData& a, const ProjectData& b) {
			return a.lastOpened > b.lastOpened;
			});

        return projects;
    }

private:
	ProjectTableView* tableView;
	ProjectOverview* projectOverview;
};
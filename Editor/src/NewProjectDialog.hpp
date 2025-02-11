#pragma once

#include <QtWidgets/qdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qboxlayout.h>

#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

struct ProjectData
{
    QString name;
    QString path;
    QDateTime lastOpened;

    bool operator==(const ProjectData& other) const
    {
        return name == other.name && path == other.path;
    }

    bool operator!=(const ProjectData& other) const
    {
        return !(*this == other);
    }

    bool operator<(const ProjectData& other) const
    {
        return lastOpened > other.lastOpened;
    }

    ProjectData(QString name, QString path, QDateTime lastOpened) : name(name), path(path), lastOpened(lastOpened) {}
};

class NewProjectDialog : public QDialog 
{
    Q_OBJECT

public:
    NewProjectDialog(QWidget* parent = nullptr) : QDialog(parent) 
    {
        setWindowTitle("Create New Project");
        setFixedWidth(400);

        QVBoxLayout* layout = new QVBoxLayout(this);

        QLabel* newProjectTitleLabel = new QLabel("New project", this);
        newProjectTitleLabel->setStyleSheet("font-size:24px; font-weight:bold;");

        projectNameEdit = new QLineEdit(this);
        projectNameEdit->setPlaceholderText("Project Name ...");

		QHBoxLayout* folderSelection = new QHBoxLayout(this);

        folderEdit = new QLineEdit(this);
		folderEdit->setPlaceholderText("Project Folder ...");
        folderSelectButton = new QPushButton("...", this);
        folderSelectButton->setFixedWidth(40);

		QHBoxLayout* buttonLayout = new QHBoxLayout(this);
        buttonLayout->setAlignment(Qt::AlignRight);
        QPushButton* createButton = new QPushButton("Create", this);
        createButton->setStyleSheet("color:#fff; background-color:#007bff; border:none; padding:4px 20px; border-radius:5px;");
        QPushButton* cancelButton = new QPushButton("Cancel", this);
        cancelButton->setStyleSheet("color:#fff; background-color:#555; border:none; padding:4px 20px; border-radius:5px;");

        layout->addWidget(newProjectTitleLabel);
        layout->addWidget(projectNameEdit);
        folderSelection->addWidget(folderEdit);
        folderSelection->addWidget(folderSelectButton);
        layout->addLayout(folderSelection);
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(createButton);
        layout->addLayout(buttonLayout);

        connect(folderSelectButton, &QPushButton::clicked, this, &NewProjectDialog::SelectFolder);
        connect(createButton, &QPushButton::clicked, this, &NewProjectDialog::CreateProject);
        connect(cancelButton, &QPushButton::clicked, this, &NewProjectDialog::reject);
    }

    QString getProjectName() const { return projectNameEdit->text(); }
    QString getProjectFolder() const { return folderEdit->text(); }

private slots:
    void SelectFolder() 
    {
        QString folder = QFileDialog::getExistingDirectory(this, "Select Project Folder");

        if (!folder.isEmpty()) 
        {
            folderEdit->setText(folder); // Set the folder path in the line edit
        }
    }

    void CreateProject() 
    {
        QString projectName = getProjectName();
        QString projectFolder = getProjectFolder();

        if (projectName.isEmpty() || projectFolder.isEmpty()) 
        {
            return;
        }

        QString projectPath = projectFolder + "/" + projectName;
		QDir().mkdir(projectPath);

		QDir().mkdir(projectPath + "/Resources");

		QFile file(projectPath + "/" + projectName + ".data");
		if (file.open(QIODevice::WriteOnly))
		{
            QJsonObject root{};
			root["projectName"] = projectName;
			root["creationDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
			file.write(QJsonDocument(root).toJson());
		}
		else
		{
			qDebug() << "Failed to create project file.";
		}

		ProjectData projectData(projectName, projectPath, QDateTime::currentDateTime());

        emit ProjectCreated(projectData);
        accept();
    }

signals:
    void ProjectCreated(const ProjectData& projectData);

private:
    QLineEdit* projectNameEdit;
    QLineEdit* folderEdit;
    QPushButton* folderSelectButton;
};
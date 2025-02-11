#pragma once

#include <QtWidgets/qdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qboxlayout.h>

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

        QVBoxLayout* layout = new QVBoxLayout(this);

        QLabel* projectNameLabel = new QLabel("Project Name:", this);
        projectNameEdit = new QLineEdit(this);

        QLabel* folderLabel = new QLabel("Project Folder:", this);
        folderEdit = new QLineEdit(this);
        folderSelectButton = new QPushButton("Choose Folder", this);

        QPushButton* createButton = new QPushButton("Create", this);
        QPushButton* cancelButton = new QPushButton("Cancel", this);

        layout->addWidget(projectNameLabel);
        layout->addWidget(projectNameEdit);
        layout->addWidget(folderLabel);
        layout->addWidget(folderEdit);
        layout->addWidget(folderSelectButton);
        layout->addWidget(createButton);
        layout->addWidget(cancelButton);

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
        qDebug() << "New project created at:" << projectPath;

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
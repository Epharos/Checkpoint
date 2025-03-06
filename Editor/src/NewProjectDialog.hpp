#pragma once

#include "pch.hpp"
#include "Widgets/Launcher/Button.hpp"

#define ANIMATION_DURATION 400

class TemplateListItem : public QWidget
{
	Q_OBJECT

protected:
	QLabel* nameLabel;
	QLabel* descriptionLabel;
	QGraphicsOpacityEffect* nameOpacityEffect;
	QGraphicsOpacityEffect* descriptionOpacityEffect;

	// TODO : Store the template project data to be used when creating a new project

public:
	TemplateListItem(const QString& name, const QString& description, QListWidget* parent = nullptr) : QWidget(parent)
	{
		nameLabel = new QLabel(name, this);
		descriptionLabel = new QLabel(description, this);
		QLabel* icon = new QLabel(this);
		icon->setPixmap(QPixmap("Editor_Resources/template.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation)); //TODO : Load template icon
		descriptionLabel->setVisible(false);

		nameOpacityEffect = new QGraphicsOpacityEffect();
		descriptionOpacityEffect = new QGraphicsOpacityEffect();

		nameLabel->setGraphicsEffect(nameOpacityEffect);
		descriptionLabel->setGraphicsEffect(descriptionOpacityEffect);

		QHBoxLayout* layout = new QHBoxLayout();
		layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		layout->addWidget(icon);
		layout->addWidget(nameLabel);
		layout->addWidget(descriptionLabel);

		setStyleSheet("padding: 0 8px;");

		setLayout(layout);
	}

	void enterEvent(QEnterEvent* event) override
	{
		QPropertyAnimation* nameAnimation = new QPropertyAnimation(nameOpacityEffect, "opacity");
		nameAnimation->setDuration(ANIMATION_DURATION);
		nameAnimation->setStartValue(1.0);
		nameAnimation->setEndValue(0.0);

		QPropertyAnimation* descriptionAnimation = new QPropertyAnimation(descriptionOpacityEffect, "opacity");
		descriptionAnimation->setDuration(ANIMATION_DURATION);
		descriptionAnimation->setStartValue(0.0);
		descriptionAnimation->setEndValue(1.0);

		nameLabel->setVisible(false);
		descriptionLabel->setVisible(true);
		nameAnimation->start();
		descriptionAnimation->start();
	}

	void leaveEvent(QEvent* event) override
	{
		QPropertyAnimation* nameAnimation = new QPropertyAnimation(nameOpacityEffect, "opacity");
		nameAnimation->setDuration(200);
		nameAnimation->setStartValue(0.0);
		nameAnimation->setEndValue(1.0);

		QPropertyAnimation* descriptionAnimation = new QPropertyAnimation(descriptionOpacityEffect, "opacity");
		descriptionAnimation->setDuration(200);
		descriptionAnimation->setStartValue(1.0);
		descriptionAnimation->setEndValue(0.0);

		nameLabel->setVisible(true);
		descriptionLabel->setVisible(false);
		nameAnimation->start();
		descriptionAnimation->start();
	}
};

class NewProjectDialog : public QDialog 
{
    Q_OBJECT

public:
    NewProjectDialog(QWidget* parent = nullptr) : QDialog(parent) 
    {
        setWindowTitle("Create New Project");
		setMinimumWidth(800);

		QHBoxLayout* layout = new QHBoxLayout(this);

		templateList = new QListWidget(this);
		templateList->setMinimumHeight(450);
		templateList->setFixedWidth(350);

		QListWidgetItem* item = new QListWidgetItem(templateList);
		item->setSizeHint(QSize(0, 64));
		item->setSelected(true);
		templateList->setItemWidget(item, new TemplateListItem("Empty Project", "An empty project with no assets."));
		item = new QListWidgetItem(templateList);
		item->setSizeHint(QSize(0, 64));
		templateList->setItemWidget(item, new TemplateListItem("3D Game", "A 3D game project with sample assets."));
		item = new QListWidgetItem(templateList);
		item->setSizeHint(QSize(0, 64));
		templateList->setItemWidget(item, new TemplateListItem("2D Game", "A 2D game project with sample assets."));

		QVBoxLayout* projectData = new QVBoxLayout();

		QLabel* projectNameLabel = new QLabel("Project Name", this);
		projectNameEdit = new QLineEdit(this);

		QLabel* folderLabel = new QLabel("Project Folder", this);
		folderEdit = new QLineEdit(this);
		QPushButton* folderSelectButton = new QPushButton("...", this);

		QHBoxLayout* folderLayout = new QHBoxLayout();
		folderLayout->addWidget(folderEdit);
		folderLayout->addWidget(folderSelectButton);

		QComboBox* graphicEngineSelection = new QComboBox(this);
		graphicEngineSelection->addItem("Vulkan");
		graphicEngineSelection->addItem("DirectX 12");
		graphicEngineSelection->setCurrentIndex(0);

		Button* createButton = new Button("Create", ButtonImportance::Primary, this);
		Button* cancelButton = new Button("Cancel", ButtonImportance::Secondary, this);

		QHBoxLayout* buttonsLayout = new QHBoxLayout();
		buttonsLayout->setAlignment(Qt::AlignRight);

		buttonsLayout->addWidget(cancelButton);
		buttonsLayout->addWidget(createButton);

		projectData->addWidget(projectNameLabel);
		projectData->addWidget(projectNameEdit);
		projectData->addWidget(folderLabel);
		projectData->addLayout(folderLayout);
		projectData->addSpacing(10);
		projectData->addWidget(graphicEngineSelection);
		projectData->addStretch();
		projectData->addLayout(buttonsLayout);

		layout->addWidget(templateList);
		layout->addLayout(projectData);

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
            folderEdit->setText(folder);
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
		QDir().mkdir(projectPath + "/Resources/Scenes");

		QFile file(projectPath + "/project.data");

		QJsonObject root{};

		if (file.open(QIODevice::WriteOnly))
		{
			root["name"] = projectName;
			root["path"] = projectPath;
			root["creationDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
			root["lastOpened"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
			root["engineVersion"] = static_cast<qint64>(ENGINE_VERSION);
			file.write(QJsonDocument(root).toJson());
		}
		else
		{
			qDebug() << "Failed to create project file.";
		}

        emit ProjectCreated(ProjectData::FromJson(root));
        accept();
    }

signals:
    void ProjectCreated(const ProjectData& projectData);

private:
	QListWidget* templateList;
	QLineEdit* projectNameEdit;
	QLineEdit* folderEdit;
};
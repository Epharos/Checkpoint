#pragma once

#include "../../pch.hpp"

#include <QtGui/qevent.h>
#include <QtCore/qmimedata.h>

template<class T>
class FileDropLineEdit : public QLineEdit
{
protected:
	std::shared_ptr<T>* resource = nullptr;
	std::string resourcePath;

	QStringList acceptedExtensions;

public:
	FileDropLineEdit(QWidget* parent = nullptr) : QLineEdit(parent)
	{
		setAcceptDrops(true);
		setReadOnly(true);
	}

	void SetAcceptedExtensions(const QStringList& _acceptedExtensions)
	{
		acceptedExtensions = _acceptedExtensions;
	}

	void SetResource(std::shared_ptr<T>* _resource)
	{
		resource = _resource;
		if(*_resource) setText(QString::fromStdString(Project::GetResourceRelativePath(cp::ResourceManager::Get()->GetResourcePath(*_resource))));
	}

	void SetResourcePath(const std::string& _resourcePath)
	{
		resourcePath = _resourcePath;
	}

protected:
	void dragEnterEvent(QDragEnterEvent* event) override
	{
		if (event->mimeData()->hasUrls())
		{
			event->acceptProposedAction();
		}
	}

	void dropEvent(QDropEvent* event) override
	{
		const QMimeData* mimeData = event->mimeData();
		if (mimeData->hasUrls())
		{
			QList<QUrl> urlList = mimeData->urls();
			if (urlList.size() > 0)
			{
				QString url = urlList.at(0).toLocalFile();
				QFileInfo fileInfo(url);

				if (fileInfo.isFile() && acceptedExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive))
				{
					setText(fileInfo.fileName());
					setToolTip(fileInfo.absoluteFilePath());

					if (resource)
					{
						*resource = cp::ResourceManager::Get()->GetOrLoad<T>(url.toStdString());
						cp::ResourceManager::Get()->GetResourceType<T>()->OptimizeMemory(resourcePath);
						resourcePath = url.toStdString();
					}
				}
			}
		}
	}
};

class MeshDropLineEdit : public FileDropLineEdit<cp::Mesh>
{
	Q_OBJECT

public:
	MeshDropLineEdit(QWidget* parent = nullptr) : FileDropLineEdit(parent)
	{
		acceptedExtensions << "obj" << "fbx" << "gltf";
	}
};
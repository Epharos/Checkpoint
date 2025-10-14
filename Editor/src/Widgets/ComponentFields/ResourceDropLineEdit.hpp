#pragma once

#include "../../pch.hpp"

#include "FileDropLineEdit.hpp"

template<class T>
class ResourceDropLineEdit : public FileDropLineEdit
{
public:
	ResourceDropLineEdit(QWidget* parent = nullptr) : FileDropLineEdit(parent)
	{
		
	}

	void SetResource(std::shared_ptr<T>* _resource)
	{
		resource = _resource;
		if (*_resource) setText(QString::fromStdString(Project::GetResourceRelativePath(cp::ResourceManager::Get()->GetResourcePath(*_resource))));
	}

protected:
	std::shared_ptr<T>* resource = nullptr;

	virtual void dropEvent(QDropEvent* event) override
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
						resourcePath = url.toStdString();
						*resource = cp::ResourceManager::Get()->GetOrLoad<T>(url.toStdString());
						cp::ResourceManager::Get()->GetResourceType<T>()->OptimizeMemory(resourcePath);
					}
				}
			}
		}
	}
};

class MeshDropLineEdit : public ResourceDropLineEdit<cp::Mesh>
{
	Q_OBJECT

public:
	MeshDropLineEdit(QWidget* parent = nullptr) : ResourceDropLineEdit(parent)
	{
		acceptedExtensions << "obj" << "fbx" << "gltf";
	}
};
#pragma once

#include "../pch.hpp"

#include "FileDropLineEdit.hpp"
#include "../Resources/Mesh.hpp"
#include "../Resources/Texture.hpp"
#include "../Resources/ResourceManager.hpp"

namespace cp
{
	template<class T>
	class ResourceDropLineEdit : public FileDropLineEdit
	{
	public:
		ResourceDropLineEdit(bool _shouldLoad = true, QWidget* parent = nullptr) : FileDropLineEdit(parent), shouldLoadResource(_shouldLoad)
		{
			setAcceptDrops(true);
			setReadOnly(true);
			setPlaceholderText("Drop a file here");
		}

		void SetResource(std::shared_ptr<T>* _resource)
		{
			resource = _resource;
			if (*_resource) setText(QString::fromStdString(GetResourcePathRelative()));
			*resourcePathOutput = cp::ResourceManager::Get()->GetResourcePath(*resource); // Reset the output path pointer
		}

		void SetResourcePathOutput(std::string* _resourcePathOutput)
		{
			resourcePathOutput = _resourcePathOutput;
		}

	protected:
		std::shared_ptr<T>* resource = nullptr;
		std::string* resourcePathOutput = nullptr; // Optional output for the resource path, can be used to get the path after setting it

		bool shouldLoadResource = true; // If true, the resource will be loaded when the path is set

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
						resourcePath = url.toStdString();

						if (resourcePathOutput)
						{
							*resourcePathOutput = resourcePath; // If the output is provided, set the resource path
						}

						if (shouldLoadResource)
						{
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
		MeshDropLineEdit(bool _shouldLoad = true, QWidget* parent = nullptr) : ResourceDropLineEdit(_shouldLoad, parent)
		{
			QStringList acceptedExtensionsTmp;
			acceptedExtensionsTmp << "obj" << "fbx" << "gltf";
			SetAcceptedExtensions(acceptedExtensionsTmp);
		}
	};

	class TextureDropLineEdit : public ResourceDropLineEdit<cp::Texture>
	{
		Q_OBJECT

	public:
		TextureDropLineEdit(bool _shouldLoad = true, QWidget* parent = nullptr) : ResourceDropLineEdit(_shouldLoad, parent)
		{
			QStringList acceptedExtensionsTmp;
			acceptedExtensionsTmp << "png" << "jpg" << "jpeg" << "tga" << "bmp" << "hdr";
			SetAcceptedExtensions(acceptedExtensionsTmp);
		}
	};
}
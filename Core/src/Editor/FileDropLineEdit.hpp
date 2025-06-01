#pragma once

#include "../pch.hpp"

#include <QtGui/qevent.h>
#include <QtCore/qmimedata.h>
#include <QtWidgets/qlineedit.h>
#include <QtCore/qurl.h>
#include <QtCore/qfileinfo.h>

namespace cp
{
	class FileDropLineEdit : public QLineEdit
	{
		Q_OBJECT

	public:
		FileDropLineEdit(QWidget* parent = nullptr) : QLineEdit(parent)
		{
			setAcceptDrops(true);
			setReadOnly(true);
		}

		void SetAcceptedExtensions(const QStringList& _acceptedExtensions)
		{
			acceptedExtensions = _acceptedExtensions;

			if (!acceptedExtensions.isEmpty())
			{
				setPlaceholderText("Drop a file here (" + acceptedExtensions.join(", ") + ")");
			}
			else
			{
				setPlaceholderText("Drop a file here");
			}
		}

		void SetResourcePath(const std::string& _resourcePath)
		{
			resourcePath = _resourcePath;
			setText(QString::fromStdString(resourcePath));
			if (!_resourcePath.empty() && _resourcePath.starts_with(Project::GetProjectPath())) setText(QString::fromStdString(GetResourcePathRelative()));
		}

		std::string GetResourcePath() const
		{
			return resourcePath;
		}

		std::string GetResourcePathRelative() const
		{
			if (resourcePath.starts_with(Project::GetProjectPath()))
				return Project::GetResourceRelativePath(resourcePath);

			return resourcePath;
		}

		std::string GetResourcePathRelative(std::string _path) const
		{
			if (_path.starts_with(Project::GetProjectPath()))
				return Project::GetResourceRelativePath(_path);

			return _path;
		}

	protected:
		std::string resourcePath;

		QStringList acceptedExtensions;

		virtual void dragEnterEvent(QDragEnterEvent* event) override
		{
			if (event->mimeData()->hasUrls())
			{
				event->acceptProposedAction();
			}
		}

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
						setToolTip(fileInfo.absoluteFilePath());

						resourcePath = url.toStdString();
						setText(QString::fromStdString(GetResourcePathRelative()));
						emit ResourcePathChanged(resourcePath);
					}
				}
			}
		}

	signals:
		void ResourcePathChanged(const std::string& _resourcePath);
	};
}
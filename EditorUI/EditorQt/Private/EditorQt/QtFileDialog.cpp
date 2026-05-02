#include "QtFileDialog.hpp"

#include <QFileDialog>
#include <QWidget>

namespace cp::editorqt
{
	QtFileDialog::QtFileDialog(std::string _id)
		: id(std::move(_id))
	{
	}

	QtFileDialog::~QtFileDialog() = default;

	std::string_view QtFileDialog::GetId() const
	{
		return id;
	}

	void QtFileDialog::SetVisible(bool _visible)
	{
		visible = _visible;
	}

	bool QtFileDialog::IsVisible() const
	{
		return visible;
	}

	void QtFileDialog::SetEnabled(bool _enabled)
	{
		enabled = _enabled;
	}

	bool QtFileDialog::IsEnabled() const
	{
		return enabled;
	}

	void QtFileDialog::SetMode(Mode _mode)
	{
		mode = _mode;
	}

	editorui::IFileDialog::Mode QtFileDialog::GetMode() const
	{
		return mode;
	}

	void QtFileDialog::SetTitle(std::string _title)
	{
		// Not directly stored but would be used in Execute()
	}

	void QtFileDialog::SetDirectory(std::filesystem::path _directory)
	{
		directory = std::move(_directory);
	}

	void QtFileDialog::SetFilter(std::string _filter)
	{
		filter = std::move(_filter);
	}

	void QtFileDialog::SetDefaultFileName(std::string _fileName)
	{
		defaultFileName = std::move(_fileName);
	}

	std::optional<std::filesystem::path> QtFileDialog::GetSelectedPath() const
	{
		if (selectedPaths.empty())
		{
			return std::nullopt;
		}
		return selectedPaths.front();
	}

	std::vector<std::filesystem::path> QtFileDialog::GetSelectedPaths() const
	{
		return selectedPaths;
	}

	bool QtFileDialog::Execute()
	{
		if (!enabled)
		{
			return false;
		}

		selectedPaths.clear();

		switch (mode)
		{
		case Mode::OpenFile:
		{
			const QString selectedPath = QFileDialog::getOpenFileName(
				nullptr,
				"Open File",
				QString::fromStdString(directory.string()),
				QString::fromStdString(filter)
			);
			if (!selectedPath.isEmpty())
			{
				selectedPaths.push_back(std::filesystem::path(selectedPath.toStdString()));
				return true;
			}
			return false;
		}
		case Mode::OpenFiles:
		{
			const QStringList selectedPathsList = QFileDialog::getOpenFileNames(
				nullptr,
				"Open Files",
				QString::fromStdString(directory.string()),
				QString::fromStdString(filter)
			);
			for (const QString& path : selectedPathsList)
			{
				selectedPaths.push_back(std::filesystem::path(path.toStdString()));
			}
			return !selectedPaths.empty();
		}
		case Mode::OpenFolder:
		{
			const QString selectedPath = QFileDialog::getExistingDirectory(
				nullptr,
				"Open Folder",
				QString::fromStdString(directory.string())
			);
			if (!selectedPath.isEmpty())
			{
				selectedPaths.push_back(std::filesystem::path(selectedPath.toStdString()));
				return true;
			}
			return false;
		}
		case Mode::SaveFile:
		{
			const QString selectedPath = QFileDialog::getSaveFileName(
				nullptr,
				"Save File",
				QString::fromStdString((directory / defaultFileName).string()),
				QString::fromStdString(filter)
			);
			if (!selectedPath.isEmpty())
			{
				selectedPaths.push_back(std::filesystem::path(selectedPath.toStdString()));
				return true;
			}
			return false;
		}
		default:
			return false;
		}
	}
}

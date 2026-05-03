#pragma once

#include "EditorQtCommon.hpp"

#include <EditorUI/FileDrop.hpp>

#include <functional>
#include <vector>

#include <QMimeData>

namespace cp::editorqt
{
	class QtFileDropWidget final : public QWidget, public cp::editorui::IFileDrop
	{
		Q_OBJECT

	public:
		explicit QtFileDropWidget(QWidget* _parent = nullptr)
			: QWidget(_parent),
			  layout(new QHBoxLayout(this)),
			  label(new QLabel("Drop mesh file here...")),
			  clearButton(new QPushButton("Clear")),
			  placeholderText("Drop mesh file here...")
		{
			setAcceptDrops(true);
			setStyleSheet("border: 2px dashed #888; border-radius: 4px; background-color: #f5f5f5;");
			setMinimumHeight(32);

			layout->setContentsMargins(8, 4, 8, 4);
			layout->setSpacing(8);
			layout->addWidget(label.get(), 1);
			layout->addWidget(clearButton.get());

			QObject::connect(clearButton.get(), &QPushButton::clicked, [this]()
			{
				Clear();
			});
		}

		// IWidget implementation
		std::string_view GetId() const override
		{
			static constexpr std::string_view id = "QtFileDropWidget";
			return id;
		}

		void SetVisible(bool _visible) override
		{
			QWidget::setVisible(_visible);
		}

		bool IsVisible() const override
		{
			return QWidget::isVisible();
		}

		void SetEnabled(bool _enabled) override
		{
			QWidget::setEnabled(_enabled);
		}

		bool IsEnabled() const override
		{
			return QWidget::isEnabled();
		}

		// IFileDrop implementation
		void SetFileExtensions(const std::vector<std::string>& _extensions) override
		{
			acceptedExtensions = _extensions;
		}

		const std::vector<std::string>& GetFileExtensions() const override
		{
			return acceptedExtensions;
		}

		void SetFileDropCallback(FileDropCallback _callback) override
		{
			onFileDropped = std::move(_callback);
		}

		void SetFilePath(const std::filesystem::path& _path) override
		{
			currentFilePath = _path;
			UpdateLabel();
		}

		[[nodiscard]] std::filesystem::path GetFilePath() const override
		{
			return currentFilePath;
		}

		void SetPlaceholderText(std::string _text) override
		{
			placeholderText = std::move(_text);
			if (currentFilePath.empty())
			{
				UpdateLabel();
			}
		}

		std::string GetPlaceholderText() const override
		{
			return placeholderText;
		}

		void Clear() override
		{
			currentFilePath.clear();
			UpdateLabel();
			if (onFileDropped)
			{
				onFileDropped(std::filesystem::path{});
			}
		}

	protected:
		void dragEnterEvent(QDragEnterEvent* _event) override
		{
			if (_event == nullptr || !_event->mimeData())
			{
				return;
			}

			if (_event->mimeData()->hasUrls())
			{
				_event->acceptProposedAction();
				setStyleSheet("border: 2px solid #4CAF50; border-radius: 4px; background-color: #e8f5e9;");
			}
		}

		void dragLeaveEvent(QDragLeaveEvent* _event) override
		{
			if (_event)
			{
				_event->accept();
				setStyleSheet("border: 2px dashed #888; border-radius: 4px; background-color: #f5f5f5;");
			}
		}

		void dropEvent(QDropEvent* _event) override
		{
			if (_event == nullptr || !_event->mimeData())
			{
				return;
			}

			setStyleSheet("border: 2px dashed #888; border-radius: 4px; background-color: #f5f5f5;");

			if (!_event->mimeData()->hasUrls())
			{
				return;
			}

			const QList<QUrl> urls = _event->mimeData()->urls();
			if (urls.isEmpty())
			{
				return;
			}

			std::filesystem::path droppedPath(urls.first().toLocalFile().toStdString());
			if (!std::filesystem::exists(droppedPath))
			{
				return;
			}

			if (!acceptedExtensions.empty())
			{
				const std::string extension = droppedPath.extension().string();
				const bool isAccepted = std::find_if(acceptedExtensions.begin(), acceptedExtensions.end(),
					[&extension](const std::string& _ext)
					{
						return _ext == extension || _ext == extension.substr(1);
					}) != acceptedExtensions.end();

				if (!isAccepted)
				{
					return;
				}
			}

			currentFilePath = droppedPath;
			UpdateLabel();
			_event->acceptProposedAction();

			if (onFileDropped)
			{
				onFileDropped(currentFilePath);
			}
		}

	private:
		void UpdateLabel()
		{
			if (currentFilePath.empty())
			{
				label->setText(ToQString(placeholderText));
			}
			else
			{
				label->setText(ToQString(currentFilePath.filename().string()));
			}
		}

		std::unique_ptr<QHBoxLayout> layout;
		std::unique_ptr<QLabel> label;
		std::unique_ptr<QPushButton> clearButton;
		std::vector<std::string> acceptedExtensions{".fbx", ".obj", ".gltf", ".glb", ".blend"};
		std::filesystem::path currentFilePath;
		std::string placeholderText;
		FileDropCallback onFileDropped;
	};
}

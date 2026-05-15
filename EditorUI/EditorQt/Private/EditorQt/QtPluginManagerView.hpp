#pragma once

#include "EditorQtCommon.hpp"

#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>

namespace cp::editorqt
{
	class QtPluginManagerView final
		: public cp::editorui::IPluginManagerView
		, public IQtWidgetAccess
		, private QtWidgetBase
	{
	public:
		explicit QtPluginManagerView(std::string _id)
			: QtWidgetBase(std::move(_id))
			, root(new QWidget())
		{
			root->setObjectName(ToQString(GetId()));

			auto* outerLayout = new QVBoxLayout(root.get());
			outerLayout->setContentsMargins(4, 4, 4, 4);
			outerLayout->setSpacing(4);

			auto* createBar = new QWidget(root.get());
			auto* createLayout = new QHBoxLayout(createBar);
			createLayout->setContentsMargins(0, 0, 0, 0);

			newPluginNameInput = new QLineEdit(createBar);
			newPluginNameInput->setPlaceholderText("New plugin name...");

			createButton = new QPushButton("Create Plugin", createBar);
			createButton->setToolTip("Scaffold a new plugin in the UserPlugins directory");

			createLayout->addWidget(newPluginNameInput, 1);
			createLayout->addWidget(createButton, 0);
			outerLayout->addWidget(createBar);

			auto* splitter = new QSplitter(Qt::Vertical, root.get());
			outerLayout->addWidget(splitter, 1);

			auto* listWidget = new QWidget(splitter);
			auto* listLayout = new QVBoxLayout(listWidget);
			listLayout->setContentsMargins(0, 0, 0, 0);
			listLayout->setSpacing(4);

			pluginList = new QListWidget(listWidget);
			pluginList->setSelectionMode(QAbstractItemView::SingleSelection);
			pluginList->setAlternatingRowColors(true);
			listLayout->addWidget(pluginList, 1);

			auto* actionBar = new QWidget(listWidget);
			auto* actionLayout = new QHBoxLayout(actionBar);
			actionLayout->setContentsMargins(0, 0, 0, 0);

			recompileButton = new QPushButton("Recompile", actionBar);
			recompileButton->setToolTip("Rebuild the selected plugin and hot-reload it");
			recompileButton->setEnabled(false);

			openFolderButton = new QPushButton("Open Folder", actionBar);
			openFolderButton->setToolTip("Open the plugin source directory in Explorer");
			openFolderButton->setEnabled(false);

			actionLayout->addWidget(recompileButton);
			actionLayout->addWidget(openFolderButton);
			actionLayout->addStretch();
			listLayout->addWidget(actionBar);
			splitter->addWidget(listWidget);

			auto* outputWidget = new QWidget(splitter);
			auto* outputLayout = new QVBoxLayout(outputWidget);
			outputLayout->setContentsMargins(0, 0, 0, 0);
			outputLayout->setSpacing(2);

			auto* outputLabel = new QLabel("Build Output", outputWidget);
			outputLabel->setStyleSheet("font-weight: bold;");
			outputLayout->addWidget(outputLabel);

			buildOutputArea = new QPlainTextEdit(outputWidget);
			buildOutputArea->setReadOnly(true);
			buildOutputArea->setLineWrapMode(QPlainTextEdit::NoWrap);

			QFont mono;
			mono.setFamily("Consolas");
			mono.setPointSize(9);
			buildOutputArea->setFont(mono);

			outputLayout->addWidget(buildOutputArea, 1);
			splitter->addWidget(outputWidget);

			splitter->setStretchFactor(0, 2);
			splitter->setStretchFactor(1, 1);

			QObject::connect(pluginList, &QListWidget::itemSelectionChanged, [this]()
			{
				const bool hasSelection = !pluginList->selectedItems().isEmpty();
				recompileButton->setEnabled(hasSelection && !SelectedPluginIsBusy());
				openFolderButton->setEnabled(hasSelection);
			});

			QObject::connect(createButton, &QPushButton::clicked, [this]()
			{
				const std::string name = ToStdString(newPluginNameInput->text().trimmed());
				if (name.empty()) return;

				if (createPluginHandler) createPluginHandler(name);

				newPluginNameInput->clear();
			});

			QObject::connect(recompileButton, &QPushButton::clicked, [this]()
			{
				const auto selected = pluginList->selectedItems();
				if (selected.isEmpty() || !recompileHandler) return;

				const std::string name = ToStdString(selected.first()->data(Qt::UserRole).toString());
				buildOutputArea->clear();

				if (recompileHandler) recompileHandler(name);
			});

			QObject::connect(openFolderButton, &QPushButton::clicked, [this]()
			{
				const auto selected = pluginList->selectedItems();
				if (selected.isEmpty() || !openFolderHandler) return;

				const std::string name = ToStdString(selected.first()->data(Qt::UserRole).toString());
				if (openFolderHandler) openFolderHandler(name);
			});
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(const bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(const bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetPlugins(std::vector<cp::editorui::IPluginManagerView::PluginEntry> entries) override
		{
			std::string selectedName;
			if (const auto* item = pluginList->currentItem())
			{
				selectedName = ToStdString(item->data(Qt::UserRole).toString());
			}

			pluginList->clear();
			pluginEntries = std::move(entries);

			for (const auto& entry : pluginEntries)
			{
				auto* item = new QListWidgetItem(FormatEntry(entry));
				item->setData(Qt::UserRole, ToQString(entry.name));
				item->setForeground(StatusColor(entry.status));
				pluginList->addItem(item);

				if (entry.name == selectedName)
				{
					pluginList->setCurrentItem(item);
				}
			}

			const bool hasSelection = !pluginList->selectedItems().isEmpty();
			recompileButton->setEnabled(hasSelection && !SelectedPluginIsBusy());
			openFolderButton->setEnabled(hasSelection);
		}

		void AppendBuildLine(std::string_view pluginName, std::string_view line) override
		{
			const std::string lineStr(line);
			const std::string nameStr(pluginName);
			QMetaObject::invokeMethod(buildOutputArea, [this, nameStr, lineStr]()
			{
				QString text = ToQString(lineStr);

				while (text.endsWith('\n') || text.endsWith('\r'))
				{
					text.chop(1);
				}

				if (!text.isEmpty())
				{
					buildOutputArea->appendPlainText(text);
				}

				QScrollBar* bar = buildOutputArea->verticalScrollBar();
				bar->setValue(bar->maximum());
			}, Qt::QueuedConnection);
		}

		void ClearBuildOutput() override
		{
			buildOutputArea->clear();
		}

		void SetRecompileHandler(RecompileHandler handler) override
		{
			recompileHandler = std::move(handler);
		}

		void SetCreatePluginHandler(CreatePluginHandler handler) override
		{
			createPluginHandler = std::move(handler);
		}

		void SetOpenFolderHandler(OpenFolderHandler handler) override
		{
			openFolderHandler = std::move(handler);
		}

		[[nodiscard]] QWidget* GetQWidget() const override { return root.get(); }

	private:
		[[nodiscard]] static QString FormatEntry(const cp::editorui::IPluginManagerView::PluginEntry& e)
		{
			const QString statusIcon = [&]
			{
				if (e.status == "Ready") return QString("✓");
				if (e.status == "Error") return QString("✗");
				if (e.status == "Building") return QString("~");
				return QString("/");
			}();

			return QString("%1  %2  (%3)")
				.arg(statusIcon)
				.arg(ToQString(e.name))
				.arg(ToQString(e.status));
		}

		[[nodiscard]] static QColor StatusColor(const std::string& status)
		{
			if (status == "Ready") return QColor(80, 200, 120);
			if (status == "Error") return QColor(230, 80, 80);
			if (status == "Building") return QColor(230, 180, 60);
			return QColor(150, 150, 150);
		}

		[[nodiscard]] bool SelectedPluginIsBusy() const
		{
			const auto selected = pluginList->selectedItems();
			if (selected.isEmpty()) return false;

			const std::string name = ToStdString(selected.first()->data(Qt::UserRole).toString());
			for (const auto& e : pluginEntries)
			{
				if (e.name == name) return e.isBusy;
			}

			return false;
		}

		std::unique_ptr<QWidget> root;

		QLineEdit* newPluginNameInput = nullptr;
		QPushButton* createButton = nullptr;
		QListWidget* pluginList = nullptr;
		QPushButton* recompileButton = nullptr;
		QPushButton* openFolderButton = nullptr;
		QPlainTextEdit* buildOutputArea = nullptr;

		std::vector<cp::editorui::IPluginManagerView::PluginEntry> pluginEntries;

		RecompileHandler recompileHandler;
		CreatePluginHandler createPluginHandler;
		OpenFolderHandler openFolderHandler;
	};
}

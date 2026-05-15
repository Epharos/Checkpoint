#pragma once

#include "EditorQtCommon.hpp"

namespace cp::editorqt
{
	class QtConsoleView final : public cp::editorui::IConsoleView, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtConsoleView(std::string _id)
			: QtWidgetBase(std::move(_id)),
			  root(new QWidget())
		{
			root->setObjectName(ToQString(GetId()));

			auto* layout = new QVBoxLayout(root.get());
			layout->setContentsMargins(0, 0, 0, 0);

			auto* topBar = new QWidget(root.get());
			auto* topBarLayout = new QHBoxLayout(topBar);
			topBarLayout->setContentsMargins(0, 0, 0, 0);

			searchBox = new QLineEdit(topBar);
			levelFilter = new QComboBox(topBar);
			searchBox->setPlaceholderText("Search logs...");
			levelFilter->addItems(QStringList{ "Trace", "Debug", "Info", "Warning", "Error", "Fatal" });
			levelFilter->setCurrentIndex(LogLevelToIndex(cp::editorui::LogLevel::Trace));

			topBarLayout->addWidget(searchBox, 1);
			topBarLayout->addWidget(levelFilter, 0);
			layout->addWidget(topBar);

			table = new QTableWidget(root.get());
			table->setColumnCount(3);
			table->setHorizontalHeaderLabels(QStringList{ "Time", "Level", "Message" });
			table->horizontalHeader()->setStretchLastSection(true);
			table->setEditTriggers(QAbstractItemView::NoEditTriggers);
			table->setSelectionBehavior(QAbstractItemView::SelectRows);
			layout->addWidget(table, 1);

			QObject::connect(searchBox, &QLineEdit::textChanged, [this](const QString& _text)
			{
				searchText = ToStdString(_text);
				ApplyFilters();
			});

			QObject::connect(levelFilter, &QComboBox::currentIndexChanged, [this](const int _index)
			{
				minLevel = IndexToLogLevel(_index);
				ApplyFilters();
			});
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(const bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(const bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetEntries(std::vector<cp::editorui::ConsoleEntry> _entries) override
		{
			entries = std::move(_entries);
			ApplyFilters();
			table->scrollToItem(table->item(table->rowCount() - 1, 0));
		}

		void SetSearchText(std::string _searchText) override
		{
			searchText = std::move(_searchText);
			searchBox->setText(ToQString(searchText));
			ApplyFilters();
		}

		void SetMinLevelFilter(const cp::editorui::LogLevel _minLevel) override
		{
			minLevel = _minLevel;
			levelFilter->setCurrentIndex(LogLevelToIndex(_minLevel));
			ApplyFilters();
		}

		void Clear() override
		{
			entries.clear();
			ApplyFilters();
		}

		[[nodiscard]] QWidget* GetQWidget() const override
		{
			return root.get();
		}

	private:
		void ApplyFilters() const
		{
			table->setRowCount(0);

			for (const cp::editorui::ConsoleEntry& entry : entries)
			{
				if (entry.level < minLevel)
				{
					continue;
				}

				if (!searchText.empty())
				{
					const QString qSearch = ToQString(searchText);
					if (!ToQString(entry.message).contains(qSearch, Qt::CaseInsensitive) &&
						!ToQString(entry.channel).contains(qSearch, Qt::CaseInsensitive))
					{
						continue;
					}
				}

				const int row = table->rowCount();
				table->insertRow(row);

				const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(entry.timestamp.time_since_epoch()).count();
				const QDateTime time = QDateTime::fromMSecsSinceEpoch(millis);

				table->setItem(row, 0, new QTableWidgetItem(time.toString("HH:mm:ss")));
				table->setItem(row, 1, new QTableWidgetItem(LogLevelToQString(entry.level)));
				table->setItem(row, 2, new QTableWidgetItem(ToQString(entry.message)));
			}
		}

		std::unique_ptr<QWidget> root;
		QLineEdit* searchBox = nullptr;
		QComboBox* levelFilter = nullptr;
		QTableWidget* table = nullptr;
		std::vector<cp::editorui::ConsoleEntry> entries;
		std::string searchText;
		cp::editorui::LogLevel minLevel = cp::editorui::LogLevel::Trace;
	};
}


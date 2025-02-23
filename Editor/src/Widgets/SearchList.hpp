#pragma once

#include "../pch.hpp"

class SearchList : public QWidget
{
	Q_OBJECT

public:
	SearchList(QWidget* parent = nullptr) : QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		searchBox = new QLineEdit(this);
		list = new QListWidget(this);

		layout->addWidget(searchBox);
		layout->addWidget(list);

		searchBox->setPlaceholderText("Search...");

		connect(searchBox, &QLineEdit::textChanged, this, &SearchList::Search);
		connect(list, &QListWidget::itemDoubleClicked, this, &SearchList::SelectItem);
	}

	void Populate(const std::unordered_map<std::type_index, std::string>& _items)
	{
		items = _items;
		list->clear();

		for (auto& [type, name] : items)
		{
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(name));
			item->setData(Qt::UserRole, QString::fromStdString(type.name()));
			list->addItem(item);
		}
	}

signals:
	void ItemSelected(std::type_index _type);

private slots:
	void Search(const QString& _text)
	{
		for (int i = 0; i < list->count(); i++)
		{
			QListWidgetItem* item = list->item(i);
			item->setHidden(!item->text().contains(_text, Qt::CaseInsensitive));
		}
	}

	void SelectItem(QListWidgetItem* _item)
	{
		std::type_index type = std::type_index(typeid(void));
		std::string typeName = _item->data(Qt::UserRole).toString().toStdString();

		for (auto& [t, name] : items)
		{
			if (t.name() == typeName)
			{
				type = t;
				break;
			}
		}

		emit ItemSelected(type);
	}

protected:
	QLineEdit* searchBox;
	QListWidget* list;
	std::unordered_map<std::type_index, std::string> items;
};
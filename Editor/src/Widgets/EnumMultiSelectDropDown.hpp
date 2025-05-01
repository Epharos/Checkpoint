#pragma once

#include "../pch.hpp"
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qstyleditemdelegate.h>
#include <QtGui/qpainter.h>

class CheckBoxItemDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);

        // Properly separate checkbox and text
        const QWidget* widget = option.widget;
        QStyle* style = widget ? widget->style() : QApplication::style();

        QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &viewOption, widget);
        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &viewOption, widget);

        painter->save();

        // Draw background
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &viewOption, painter, widget);

        // Draw checkmark
        QStyleOptionViewItem checkOpt(viewOption);
        checkOpt.rect = checkRect;
        checkOpt.state = viewOption.state;
        checkOpt.state |= index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked
            ? QStyle::State_On
            : QStyle::State_Off;
        style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &checkOpt, painter, widget);

        // Draw text
        QRect actualTextRect = textRect.adjusted(20, 0, 0, 0); // Add small spacing
        painter->setPen(viewOption.palette.color(QPalette::Text));
        painter->drawText(actualTextRect, viewOption.displayAlignment, viewOption.text);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(size.height() + 4); // add extra padding if needed
        return size;
    }
};

class CheckableComboBox : public QComboBox {
    Q_OBJECT

public:
    explicit CheckableComboBox(QWidget* parent = nullptr)
        : QComboBox(parent) {
        setModel(new QStandardItemModel(this));
        view()->setEditTriggers(QAbstractItemView::NoEditTriggers);
		view()->setItemDelegate(new CheckBoxItemDelegate(this));
        setEditable(true);
        lineEdit()->setReadOnly(true);
        connect(static_cast<QStandardItemModel*>(model()), &QStandardItemModel::itemChanged, this, &CheckableComboBox::onItemChanged);
		setStyleSheet(R"(
            QComboBox {
                background-color: #444;
            }

            QComboBox QAbstractItemView {
                padding-left: 4px;
                show-decoration-selected: 1;
                outline: 0;
            }
        )");
    }

    void AddCheckItem(const QString& text, Qt::CheckState state = Qt::Unchecked) {
        QStandardItem* item = new QStandardItem(text);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(state, Qt::CheckStateRole);
        static_cast<QStandardItemModel*>(model())->appendRow(item);
    }

    void AddCheckItems(const QStringList& texts, const QList<Qt::CheckState>& states) {
        for (int i = 0; i < texts.size(); ++i) {
            AddCheckItem(texts[i], i < states.size() ? states[i] : Qt::Unchecked);
        }
    }

	void RemoveCheckItem(const QString& text) {
		QStandardItemModel* m = static_cast<QStandardItemModel*>(model());
		for (int i = 0; i < m->rowCount(); ++i) {
			QStandardItem* item = m->item(i);
			if (item->text() == text) {
				m->removeRow(i);
				break;
			}
		}
	}

	void RemoveAllCheckItems() {
		QStandardItemModel* m = static_cast<QStandardItemModel*>(model());
		m->removeRows(0, m->rowCount());
	}

    QStringList CheckedItems() const {
        QStringList items;
        QStandardItemModel* m = static_cast<QStandardItemModel*>(model());
        for (int i = 0; i < m->rowCount(); ++i) {
            QStandardItem* item = m->item(i);
            if (item->checkState() == Qt::Checked)
                items << item->text();
        }
        return items;
    }

	QList<Qt::CheckState> CheckedStates() const {
		QList<Qt::CheckState> states;
		QStandardItemModel* m = static_cast<QStandardItemModel*>(model());
		for (int i = 0; i < m->rowCount(); ++i) {
			QStandardItem* item = m->item(i);
			states << item->checkState();
		}
		return states;
	}

	bool IsChecked(const QString& text) const {
		QStandardItemModel* m = static_cast<QStandardItemModel*>(model());
		for (int i = 0; i < m->rowCount(); ++i) {
			QStandardItem* item = m->item(i);
			if (item->text() == text)
				return item->checkState() == Qt::Checked;
		}
		return false;
	}

private slots:
    void onItemChanged(QStandardItem* item) {
        Q_UNUSED(item);
        // Update the display text
        lineEdit()->setText(CheckedItems().join(", "));
    }
};
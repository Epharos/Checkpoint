#pragma once

#include <QWidget>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QRegularExpression>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QProxyStyle>
#include <QMouseEvent>
#include <vector>
#include <functional>

namespace cp {
	struct Project;

	class FilterProxyModel : public QSortFilterProxyModel
	{
	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
		{
			QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
			return sourceModel()->data(index).toString().contains(filterRegularExpression());
		}
	};


	class NoFocusFrameStyle : public QProxyStyle
	{
	public:
		using QProxyStyle::QProxyStyle;

		void drawPrimitive(PrimitiveElement element,
			const QStyleOption* option,
			QPainter* painter,
			const QWidget* widget = nullptr) const override
		{
			switch (element)
			{
			case PE_PanelItemViewRow:
			case PE_PanelItemViewItem:
				// NE RIEN PEINDRE → supprime fond bleu/gris
				return;

			case PE_FrameFocusRect:
				// supprime le cadre de focus bleu
				return;

			default:
				break;
			}

			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}
	};

	class ProjectListItemDelegate : public QStyledItemDelegate
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	public:
		explicit ProjectListItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			const QWidget* widget = option.widget;
			const QAbstractItemView* view =
				qobject_cast<const QAbstractItemView*>(widget);

			bool isSelected = option.state & QStyle::State_Selected;
			bool isHovered = option.state & QStyle::State_MouseOver;

			QStyleOptionViewItem opt(option);
			opt.backgroundBrush = Qt::NoBrush;          // IMPORTANT !
			opt.state &= ~QStyle::State_Selected;       // empêche background bleu
			opt.state &= ~QStyle::State_MouseOver;      // empêche hover par cellule
			opt.state &= ~QStyle::State_HasFocus;

			opt.features &= ~QStyleOptionViewItem::HasDisplay;
			opt.features &= ~QStyleOptionViewItem::Alternate;
			opt.features &= ~QStyleOptionViewItem::HasCheckIndicator;
			opt.features &= ~QStyleOptionViewItem::HasDecoration;

			painter->save();

			int radius = 6;

			//----------------------------------------
			// 1) DESSIN DE LA LIGNE COMPLETE (colonne 0)
			//----------------------------------------
			if (index.column() == 0 && view && isSelected)
			{
				QRect rowRect(0, option.rect.top(),
					view->viewport()->width(),
					option.rect.height());

				painter->setRenderHint(QPainter::Antialiasing);
				painter->setBrush(QColor(0, 0, 255));
				painter->setPen(Qt::NoPen);

				painter->drawRoundedRect(rowRect.adjusted(2, 1, -2, -1), radius, radius);
			}

			painter->restore();

			//----------------------------------------
			// 2) DESSIN NORMAL DU TEXTE DE CHAQUE CELL
			//----------------------------------------
			QStyledItemDelegate::paint(painter, opt, index);

			//----------------------------------------
			// 3) AJOUT DU SOUS-TEXTE (TOOLTIP) dans la 1�re colonne
			//----------------------------------------
			//if (index.column() == 0 && isSelected)
			//{
			//	QString tooltip = index.data(Qt::ToolTipRole).toString();
			//	if (!tooltip.isEmpty())
			//	{
			//		painter->save();
			//		painter->setPen(QColor(80, 80, 80));

			//		QRect textRect = option.rect.adjusted(4, 18, -4, -2);

			//		QFont small = option.font;
			//		small.setPointSize(small.pointSize() - 2);
			//		painter->setFont(small);

			//		painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, tooltip);

			//		painter->restore();
			//	}
			//}
		}
	};


	class ProjectListTreeWidget : public QTableView
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
		public:
			explicit ProjectListTreeWidget(QWidget* _parent = nullptr) : QTableView(_parent)
			{
				setItemDelegate(new ProjectListItemDelegate(this));
				setStyle(new NoFocusFrameStyle(style()));
			}
	};


	class ProjectList : public QWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	protected:
		ProjectListTreeWidget* projectListWidget;
		QStandardItemModel* model;
		FilterProxyModel* proxyModel;

		QLineEdit* searchBox;

	public:
		ProjectList(QWidget* _parent = nullptr);
		void PopulateProjectList(const std::vector<cp::Project>& _projects);
		void AddProject(const size_t _index, const cp::Project& _project);

		void AddProjectFocusedListener(std::function<void(const std::string&)> _callback);
		void AddProjectOpenedListener(std::function<void(const std::string&)> _callback);

	signals:
		void ProjectFocused(const std::string& _projectPath);
		void ProjectOpened(const std::string& _projectPath);

	private slots:
		void SelectProject(const QModelIndex& _index);
		void OpenProject(const QModelIndex& _index);
	};
}
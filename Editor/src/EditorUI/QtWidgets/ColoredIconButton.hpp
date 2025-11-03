#pragma once

#include <QtWidgets/qpushbutton.h>
#include <QtGui/qcolor.h>
#include <QtGui/qevent.h>
#include "Helper.hpp"
#include <optional>

namespace cp {
	class ColoredIconButton : public QPushButton
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	public:
		ColoredIconButton(const QString& iconPath, const QSize& iconSize, const QColor& color, QWidget* parent = nullptr)
			: QPushButton(parent), iconPath(iconPath), iconSize(iconSize), color(color)
		{
			QPixmap pixmap = SvgToPixmap(iconPath, iconSize, color);
			setIcon(QIcon(pixmap));
			setFlat(true);
			setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
			setIconSize(iconSize);
			setStyleSheet("QPushButton { border: none; background: transparent; padding: 4px 0; }");
		}

		void setHoveredColor(const QColor& hoveredColor) {
			this->hoveredColor = hoveredColor;
		}

		void setDisabledColor(const QColor& disabledColor) {
			this->disabledColor = disabledColor;

			if (!isEnabled()) {
				QPixmap pixmap = SvgToPixmap(iconPath, iconSize, disabledColor);
				setIcon(QIcon(pixmap));
			}
		}

		void enterEvent(QEnterEvent* event) override {
			if (isEnabled() && hoveredColor.has_value()) {
				QPixmap pixmap = SvgToPixmap(iconPath, iconSize, hoveredColor.value());
				setIcon(QIcon(pixmap));
			}

			QPushButton::enterEvent(event);
		}

		void leaveEvent(QEvent* event) override {
			if (isEnabled()) {
				QPixmap pixmap = SvgToPixmap(iconPath, iconSize, color);
				setIcon(QIcon(pixmap));
			}
			else
			{
				QPixmap pixmap = SvgToPixmap(iconPath, iconSize, disabledColor.has_value() ? disabledColor.value() : color);
				setIcon(QIcon(pixmap));
			}

			QPushButton::leaveEvent(event);
		}

	protected:
		QString iconPath;
		QSize iconSize;
		QColor color;
		std::optional<QColor> hoveredColor;
		std::optional<QColor> disabledColor;
	};
};
#pragma once

#include <QtSvg/qsvgrenderer.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qpainter.h>

namespace cp
{
	QPixmap SvgToPixmap(const QString& _svgPath, const QSize& _size, const QColor& _color);
}
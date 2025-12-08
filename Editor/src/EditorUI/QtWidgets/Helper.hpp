#pragma once

#include <QtSvg/qsvgrenderer.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qpainter.h>
#include <cstdint>

namespace cp
{
	QPixmap SvgToPixmap(const QString& _svgPath, const QSize& _size, const QColor& _color);
	uint32_t HexColorToUInt32(const std::string& _colorHex);
}
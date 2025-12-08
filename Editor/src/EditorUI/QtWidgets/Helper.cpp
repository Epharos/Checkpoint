#include "Helper.hpp"

namespace cp
{
	QPixmap SvgToPixmap(const QString& _svgPath, const QSize& _size, const QColor& _color)
	{
		QSvgRenderer svgRenderer(_svgPath);
		QPixmap pixmap(_size);
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		svgRenderer.render(&painter);

		if (_color.isValid())
		{
			painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
			painter.fillRect(pixmap.rect(), _color);
			painter.end();
		}

		return pixmap;
	}

	uint32_t HexColorToUInt32(const std::string& _colorHex)
	{
		std::string hex = _colorHex;

		if (hex[0] == '#')
		{
			hex = hex.substr(1);
		}

		uint32_t colorValue = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));

		if (hex.length() == 6)
		{
			colorValue |= 0xFF000000; // Add full alpha if not specified
		}

		return colorValue;
	}
}
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
}
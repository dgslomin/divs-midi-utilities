
// This exists solely as a styling workaround.

#include <QtWidgets>
#include "colors.h"
#include "splitter.h"

Splitter::Splitter(Qt::Orientation orientation, QColor color, QWidget* parent): QSplitter(orientation, parent)
{
	this->pen = QPen(color);
}

QSplitterHandle* Splitter::createHandle()
{
	return new SplitterHandle(this->orientation(), this->pen, this);
}

SplitterHandle::SplitterHandle(Qt::Orientation orientation, QPen pen, QSplitter* parent): QSplitterHandle(orientation, parent)
{
	this->pen = pen;
}

void SplitterHandle::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.setPen(this->pen);

	if (this->orientation() == Qt::Horizontal)
	{
		painter.drawLine(this->width() / 2, 0, this->width() / 2, this->height());
	}
	else
	{
		painter.drawLine(0, this->height() / 2, this->width(), this->height() / 2);
	}
}


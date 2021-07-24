
// This exists solely as a styling workaround.

#include <QtWidgets>
#include "colors.h"
#include "splitter.h"

Splitter::Splitter(Qt::Orientation orientation, QWidget* parent): QSplitter(orientation, parent)
{
}

Splitter::Splitter(QWidget *parent): QSplitter(parent)
{
}

QSplitterHandle* Splitter::createHandle()
{
	return new SplitterHandle(this->orientation(), this);
}

SplitterHandle::SplitterHandle(Qt::Orientation orientation, QSplitter* parent): QSplitterHandle(orientation, parent)
{
	this->pen = QPen(Colors::buttonShade(220, 70));
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



// This exists solely as a styling workaround for QFrame's line mode.

#include <QtWidgets>
#include "colors.h"
#include "separator.h"

Separator::Separator(Qt::Orientation orientation, QWidget* parent): QWidget(parent)
{
	this->orientation = orientation;
	this->pen = QPen(Colors::buttonShade(220, 70));
	this->setMinimumSize(1, 1);
}

void Separator::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.setPen(this->pen);

	if (this->orientation == Qt::Horizontal)
	{
		painter.drawLine(0, this->height() / 2, this->width(), this->height() / 2);
	}
	else
	{
		painter.drawLine(this->width() / 2, 0, this->width() / 2, this->height());
	}
}


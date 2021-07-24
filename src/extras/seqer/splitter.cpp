
// This exists solely as a styling workaround.

#include <QtWidgets>
#include "splitter.h"

Splitter::Splitter(Qt::Orientation orientation, QWidget* parent): QSplitter(orientation, parent)
{
	this->setHandleWidth(1);
}

Splitter::Splitter(QWidget *parent): QSplitter(parent)
{
	this->setHandleWidth(1);
}

QSplitterHandle* Splitter::createHandle()
{
	return new SplitterHandle(this->orientation(), this);
}

SplitterHandle::SplitterHandle(Qt::Orientation orientation, QSplitter* parent): QSplitterHandle(orientation, parent)
{
	this->color = QGuiApplication::palette().color(QPalette::Mid);
}

void SplitterHandle::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.fillRect(this->rect(), this->color);
}


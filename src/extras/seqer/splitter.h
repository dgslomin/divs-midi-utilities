#ifndef SPLITTER_INCLUDED
#define SPLITTER_INCLUDED

class Splitter;
class SplitterHandle;

#include <QtWidgets>

class Splitter: public QSplitter
{
	Q_OBJECT

public:
	QPen pen;

	Splitter(Qt::Orientation orientation, QColor color, QWidget* parent = nullptr);

protected:
	QSplitterHandle* createHandle();
};

class SplitterHandle: public QSplitterHandle
{
	Q_OBJECT

public:
	QPen pen;

	SplitterHandle(Qt::Orientation orientation, QPen pen, QSplitter* parent);

protected:
	void paintEvent(QPaintEvent* event);
};

#endif

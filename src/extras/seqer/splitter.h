#ifndef SPLITTER_INCLUDED
#define SPLITTER_INCLUDED

class Splitter;
class SplitterHandle;

#include <QtWidgets>

class Splitter: public QSplitter
{
	Q_OBJECT

public:
	Splitter(Qt::Orientation orientation, QWidget* parent = nullptr);
	Splitter(QWidget* parent = nullptr);

protected:
	QSplitterHandle* createHandle();
};

class SplitterHandle: public QSplitterHandle
{
	Q_OBJECT

public:
	QColor color;

	SplitterHandle(Qt::Orientation orientation, QSplitter* parent);

protected:
	void paintEvent(QPaintEvent* event);
};

#endif

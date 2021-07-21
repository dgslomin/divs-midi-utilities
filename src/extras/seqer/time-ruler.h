#ifndef TIME_RULER_INCLUDED
#define TIME_RULER_INCLUDED

class TimeRuler;

#include <QtWidgets>
#include "window.h"

class TimeRuler: public QWidget
{
	Q_OBJECT

public:
	Window* window;

	TimeRuler(Window* window);

public slots:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
};

#endif

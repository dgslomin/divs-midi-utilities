#ifndef SEPARATOR_INCLUDED
#define SEPARATOR_INCLUDED

class Separator;

#include <QtWidgets>

class Separator: public QWidget
{
	Q_OBJECT

public:
	QPen pen;
	Qt::Orientation orientation;

	Separator(Qt::Orientation orientation, QColor color, QWidget* parent = nullptr);

protected:
	void paintEvent(QPaintEvent* event);
};

#endif

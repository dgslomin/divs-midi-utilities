
#include <QtWidgets>

class TouchWidget: public QWidget
{
	Q_OBJECT

public:
	TouchWidget();
	virtual bool event(QEvent* event);
	virtual void touchEvent(QTouchEvent* event);
};

class PianoWidget: public TouchWidget
{
	Q_OBJECT

public:
	PianoWidget();
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);
	int getNoteForXY(int x, int y);

	int full_width = INT_MIN;
	int pan = INT_MAX;
};



#include <QtWidgets>

class TouchPanel: public QWidget
{
	Q_OBJECT

public:
	TouchPanel();
	bool event(QEvent* event);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);
};



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
	int getBendAmountForXOffset(int x_offset);
	void noteOn(int finger_id, int note);
	void bend(int finger_id, int amount);
	void noteOff(int finger_id);

	int full_width = INT_MIN;
	int pan = INT_MAX;
	QVector<int> available_channels = QVector<int>(16);
	QVector<int> busy_channels = QVector<int>(16);
	QHash<int, int> finger_id_to_channel;
	int channel_to_finger_id[16];
	int channel_to_note[16];
};



class MidiOut;
class TouchWidget;
class PianoWidget;
class SliderWidget;
class ShiftButton;
class LatchButton;
class Window;

#include <QtWidgets>
#include <rtmidi_c.h>

class MidiOut: public QObject
{
	Q_OBJECT

public:
	static MidiOut* open(QString port_name);

private:
	MidiOut(RtMidiOutPtr underlying_midi_out);

public:
	~MidiOut();
	void noteOff(int channel, int note, int velocity);
	void noteOn(int channel, int note, int velocity);
	void controlChange(int channel, int number, int value);
	void pitchWheel(int channel, int amount);
	void mpeNoteOff(int finger_id);
	void mpeNoteOn(int finger_id, int note);
	void mpePitchWheel(int finger_id, int amount);
	void mpeAllNotesOff();

	RtMidiOutPtr underlying_midi_out;
	QVector<int> idle_channels;
	QVector<int> busy_channels;
	QHash<int, int> finger_id_to_channel;
	int channel_to_finger_id[16];
	int channel_to_note[16];
};

class TouchWidget: public QWidget
{
	Q_OBJECT

public:
	TouchWidget(Window* window);
	virtual bool event(QEvent* event);
	virtual void touchEvent(QTouchEvent* event);

	Window* window;
};

class PianoWidget: public TouchWidget
{
	Q_OBJECT

public:
	PianoWidget(Window* window);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);
	int getNote(int x, int y);
	int getPitchWheelAmount(int start_x, int start_y, int x, int y);
	float getNaturalNumber(int x);
	float getAccidentalNumber(int x);
	float getNaturalNote(float natural_number);
	float getAccidentalNote(float accidental_number);

public slots:
	void setAdjustRange(bool adjust_range);
	void setGlide(bool glide);

public:
	int full_width;
	int pan;
	bool adjust_range = false;
	bool glide = false;
	bool glissando = true;
};

class SliderWidget: public TouchWidget
{
	Q_OBJECT

public:
	SliderWidget(Window* window, QString label, int controller_number);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);

	QString label;
	int controller_number;
	float value = 0;
};

class ShiftButton: public TouchWidget
{
	Q_OBJECT

public:
	ShiftButton(Window* window, QString label);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);

	QString label;
	bool is_pressed = false;

signals:
	void stateChanged(bool is_pressed);
};

class LatchButton: public TouchWidget
{
	Q_OBJECT

public:
	LatchButton(Window* window, QString label);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);

	QString label;
	bool is_pressed = false;

signals:
	void stateChanged(bool is_pressed);
};

class Window: public QMainWindow
{
	Q_OBJECT

public:
	Window(MidiOut* midi_out);
	void closeEvent(QCloseEvent* event);

public slots:
	void toggleFullscreen();

public:
	MidiOut* midi_out;
	PianoWidget* upper_keyboard;
	PianoWidget* lower_keyboard;
};


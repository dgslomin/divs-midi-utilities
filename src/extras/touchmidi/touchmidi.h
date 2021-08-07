
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
	int getPitchWheelAmountForXOffset(int x_offset);

	int full_width = 1920 * 4;
	int pan = 1920 * 2;
};


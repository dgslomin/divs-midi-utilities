
class MidiOut;
class TouchWidget;
class PianoWidget;
class SliderWidget;
class Button;
class ShiftButton;
class LatchButton;
class Window;
class SetupDialog;

#include <QtWidgets>
#include <rtmidi_c.h>

class MidiOut: public QObject
{
	Q_OBJECT

public:
	static QVector<QString> getPortNames();
	static MidiOut* open(QString port_name);

private:
	MidiOut(QString port_name, RtMidiOutPtr underlying_midi_out);

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

	QString port_name;
	RtMidiOutPtr underlying_midi_out;
	QVector<int> idle_channels;
	QVector<int> busy_channels;
	QHash<int, int> finger_id_to_channel;
	int channel_to_finger_id[16];
	int channel_to_note[16];
	int velocity;
	int transpose;
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
	class FingerState
	{
	public:
		int x;
		int y;
		int start_y;
		int start_note;
		float note;
		float auto_aim_amount;
		QElapsedTimer settle_duration;
	};

	PianoWidget(Window* window);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);
	void timerEvent(QTimerEvent* event);
	int getStartNote(int x, int y, float& auto_aim_amount);
	float getGlideNote(int x, int y, int start_y, float auto_aim_amount);
	float getSettleNote(int x, int y, int start_y, float& auto_aim_amount);
	float getSettleAutoAimIncrement(float key_number);
	bool isNatural(int y);
	float getAccidentalFraction(int y, int start_y);
	float getNaturalNumber(int x);
	float getAccidentalNumber(int x);
	float getNaturalNote(float natural_number);
	float getAccidentalNote(float accidental_number);
	float interpolate(float value1, float value2, float fraction);
	int getPitchWheelAmount(float note_offset);

public slots:
	void setAdjustRange(bool adjust_range);
	void setGlide(bool glide);

public:
	int full_width;
	int pan;
	bool adjust_range = false;
	bool glide = false;
	QHash<int, FingerState> finger_states;
};

class SliderWidget: public TouchWidget
{
	Q_OBJECT

public:
	SliderWidget(Window* window, QString label);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);

public slots:
	void setControllerNumber(int controller_number);

public:
	QString label;
	int controller_number = -1;
	float value = 0;
};

class Button: public TouchWidget
{
	Q_OBJECT

public:
	Button(Window* window, QString label);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);

signals:
	void triggered();

public:
	QString label;
};

class ShiftButton: public TouchWidget
{
	Q_OBJECT

public:
	ShiftButton(Window* window, QString label);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);

signals:
	void stateChanged(bool is_pressed);

public:
	QString label;
	bool is_pressed = false;
};

class LatchButton: public TouchWidget
{
	Q_OBJECT

public:
	LatchButton(Window* window, QString label);
	void paintEvent(QPaintEvent* event);
	void touchEvent(QTouchEvent* event);
	void setPressed(bool is_pressed);

signals:
	void stateChanged(bool is_pressed);

public:
	QString label;
	bool is_pressed = false;
};

class Window: public QMainWindow
{
	Q_OBJECT

public:
	Window();
	void closeEvent(QCloseEvent* event);
	void setMidiOut(QString port_name);

public slots:
	void showSetupDialog();
	void toggleFullscreen();

public:
	MidiOut* midi_out = NULL;
	SliderWidget* sliders[16];
	Button* setup_button;
	ShiftButton* range_button;
	LatchButton* glide_button;
	PianoWidget* upper_keyboard;
	PianoWidget* lower_keyboard;
};

class SetupDialog: public QDialog
{
	Q_OBJECT

public:
	SetupDialog(Window* window);

	Window* window;
	QComboBox* midi_output_port_combo_box;
	QSpinBox* velocity_spin_box;
	QSpinBox* transpose_spin_box;
	QComboBox* slider_controller_number_combo_boxes[16];
};


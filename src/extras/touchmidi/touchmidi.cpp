
#include <QtWidgets>
#include <rtmidi_c.h>
#include "midiutil-common.h"
#include "midiutil-rtmidi.h"
#include "touchmidi.h"

static MidiOut* midi_out;

MidiOut* MidiOut::open(QString port_name)
{
	RtMidiOutPtr underlying_midi_out;
	if ((underlying_midi_out = rtmidi_open_out_port((char *)("touchmidi"), (char *)(port_name.toStdString().c_str()), (char *)("touchmidi"))) == NULL) return NULL;
	return new MidiOut(underlying_midi_out);
}

MidiOut::MidiOut(RtMidiOutPtr underlying_midi_out)
{
	this->underlying_midi_out = underlying_midi_out;

	// skip channel 0 since it's the zone leader
	for (int channel = 1; channel < 16; channel++) this->idle_channels.append(channel);
}

MidiOut::~MidiOut()
{
	rtmidi_close_port(this->underlying_midi_out);
}

void MidiOut::noteOff(int channel, int note, int velocity)
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
	MidiUtilMessage_setNoteOff(message, channel, note, velocity);
	rtmidi_out_send_message(this->underlying_midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
}

void MidiOut::noteOn(int channel, int note, int velocity)
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
	MidiUtilMessage_setNoteOn(message, channel, note, velocity);
	rtmidi_out_send_message(this->underlying_midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
}

void MidiOut::controlChange(int channel, int number, int value)
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE];
	MidiUtilMessage_setControlChange(message, channel, number, value);
	rtmidi_out_send_message(this->underlying_midi_out, message, MIDI_UTIL_MESSAGE_SIZE_CONTROL_CHANGE);
}

void MidiOut::pitchWheel(int channel, int amount)
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_PITCH_WHEEL];
	MidiUtilMessage_setPitchWheel(message, channel, amount);
	rtmidi_out_send_message(this->underlying_midi_out, message, MIDI_UTIL_MESSAGE_SIZE_PITCH_WHEEL);
}

void MidiOut::mpeNoteOff(int finger_id)
{
	int channel = this->finger_id_to_channel.value(finger_id, -1);
	if (channel < 0) return;
	int note = this->channel_to_note[channel];
	this->finger_id_to_channel.remove(finger_id);
	this->busy_channels.removeOne(channel);
	this->idle_channels.append(channel);
	this->noteOff(channel, note, 0);
}

/**
 * This is an MPE channel allocation algorithm that forces a single note per
 * channel with newest priority.
 */
void MidiOut::mpeNoteOn(int finger_id, int note)
{
	int channel;

	if (this->idle_channels.empty())
	{
		channel = this->busy_channels.takeFirst();
		this->finger_id_to_channel.remove(this->channel_to_finger_id[channel]);
		this->noteOff(channel, this->channel_to_note[channel], 0);
	}
	else
	{
		channel = this->idle_channels.takeFirst();
	}

	this->busy_channels.append(channel);
	this->finger_id_to_channel.insert(finger_id, channel);
	this->channel_to_finger_id[channel] = finger_id;
	this->channel_to_note[channel] = note;

	// reset pitch wheel value and range
	this->pitchWheel(channel, 1 << 13);
	this->controlChange(channel, 101, 0);
	this->controlChange(channel, 100, 0);
	this->controlChange(channel, 6, 48);
	this->controlChange(channel, 38, 0);

	this->noteOn(channel, note, 127);
}

void MidiOut::mpePitchWheel(int finger_id, int amount)
{
	int channel = this->finger_id_to_channel.value(finger_id, -1);
	if (channel < 0) return;
	this->pitchWheel(channel, amount);
}

TouchWidget::TouchWidget()
{
	this->setAttribute(Qt::WA_AcceptTouchEvents, true);
}

bool TouchWidget::event(QEvent* event)
{
	switch (event->type())
	{
		case QEvent::TouchBegin:
		case QEvent::TouchUpdate:
		case QEvent::TouchEnd:
		{
			this->touchEvent(static_cast<QTouchEvent*>(event));
			return true;
		}
		default:
		{
			return QWidget::event(event);
		}
	}
}

void TouchWidget::touchEvent(QTouchEvent* event)
{
	Q_UNUSED(event)
}

PianoWidget::PianoWidget(): TouchWidget()
{
}

void PianoWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	this->full_width = qMax(this->full_width, this->width());
	this->pan = qMin(this->pan, this->full_width - this->width());

	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), Qt::white);

	int full_number_of_notes = 128;
	int number_of_notes_per_octave = 12;
	int full_number_of_naturals = 75;
	int number_of_naturals_per_octave = 7;
	bool note_is_accidental[] = { false, true, false, true, false, false, true, false, true, false, true, false };
	float natural_width = (float)(this->full_width) / full_number_of_naturals;
	float accidental_width = natural_width * number_of_naturals_per_octave / number_of_notes_per_octave;

	painter.setPen(Qt::black);

	for (int natural_number = 1; natural_number < full_number_of_naturals; natural_number++)
	{
		float x = natural_number * natural_width - this->pan;
		painter.drawLine(x, 0, x, this->height());
	}

	for (int note_number = 0; note_number < full_number_of_notes; note_number++)
	{
		float x = note_number * accidental_width - this->pan;
		if (note_is_accidental[note_number % number_of_notes_per_octave]) painter.fillRect(x, 0, accidental_width, this->height() / 2, Qt::black);
	}
}

void PianoWidget::touchEvent(QTouchEvent* event)
{
	event->accept();

	for (const QTouchEvent::TouchPoint& touch_point: event->touchPoints())
	{
		switch (touch_point.state())
		{
			case Qt::TouchPointPressed:
			{
				QPointF pos = touch_point.pos();
				int note = this->getNote(pos.x(), pos.y());
				midi_out->mpeNoteOn(touch_point.id(), note);
				break;
			}
			case Qt::TouchPointMoved:
			{
				QPointF start_pos = touch_point.startPos();
				QPointF pos = touch_point.pos();
				int amount = this->getPitchWheelAmount(start_pos.x(), start_pos.y(), pos.x(), pos.y());
				midi_out->mpePitchWheel(touch_point.id(), amount);
				break;
			}
			case Qt::TouchPointReleased:
			{
				midi_out->mpeNoteOff(touch_point.id());
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

int PianoWidget::getNote(int x, int y)
{
	if (y > this->height() / 2)
	{
		return (int)(this->getNaturalNote((int)(this->getNaturalNumber(x))));
	}
	else
	{
		return (int)(this->getAccidentalNote((int)(this->getAccidentalNumber(x))));
	}
}

int PianoWidget::getPitchWheelAmount(int start_x, int start_y, int x, int y)
{
	int full_pitch_wheel_range = 1 << 14;
	int full_pitch_wheel_range_notes = 96;
	int note_height = this->height() / 2;
	float natural_note_offset;
	float accidental_note_offset;
	float accidental_fraction;

	if (start_y > note_height)
	{
		float start_natural_number = this->getNaturalNumber(start_x);
		float start_natural_note = this->getNaturalNote((int)(start_natural_number));
		float natural_number = this->getNaturalNumber(x);
		float natural_note = this->getNaturalNote(natural_number - (start_natural_number - (int)(start_natural_number)));
		natural_note_offset = natural_note - start_natural_note;

		float start_accidental_number = this->getAccidentalNumber(start_x);
		float start_accidental_note = this->getAccidentalNote((int)(start_accidental_number));
		float accidental_number = this->getAccidentalNumber(x);
		float accidental_note = this->getAccidentalNote(accidental_number - 0.5);
		accidental_note_offset = accidental_note - start_accidental_note;

		accidental_fraction = (float)(qMin(qMax(start_y - y, 0), note_height)) / note_height;
	}
	else
	{
		float start_natural_number = this->getNaturalNumber(start_x);
		float start_natural_note = this->getNaturalNote((int)(start_natural_number));
		float natural_number = this->getNaturalNumber(x);
		float natural_note = this->getNaturalNote(natural_number - 0.5);
		natural_note_offset = natural_note - start_natural_note;

		float start_accidental_number = this->getAccidentalNumber(start_x);
		float start_accidental_note = this->getAccidentalNote((int)(start_accidental_number));
		float accidental_number = this->getAccidentalNumber(x);
		float accidental_note = this->getAccidentalNote(accidental_number - (start_accidental_number - (int)(start_accidental_number)));
		accidental_note_offset = accidental_note - start_accidental_note;

		accidental_fraction = 1.0 - ((float)(qMin(qMax(y - start_y, 0), note_height)) / note_height);
	}

	float note_offset = (accidental_note_offset - natural_note_offset) * accidental_fraction + natural_note_offset;
	return (int)(note_offset * full_pitch_wheel_range / full_pitch_wheel_range_notes) + (1 << 13);
}

float PianoWidget::getNaturalNumber(int x)
{
	int full_number_of_notes = 128;
	int number_of_notes_per_octave = 12;
	int number_of_naturals_per_octave = 7;
	float accidental_width = (float)(this->full_width) / full_number_of_notes;
	float natural_width = accidental_width * number_of_notes_per_octave / number_of_naturals_per_octave;
	return (x + this->pan) / natural_width;
}

float PianoWidget::getAccidentalNumber(int x)
{
	int full_number_of_notes = 128;
	float accidental_width = (float)(this->full_width) / full_number_of_notes;
	return (x + this->pan) / accidental_width;
}

float PianoWidget::getNaturalNote(float natural_number)
{
	int number_of_notes_per_octave = 12;
	int number_of_naturals_per_octave = 7;
	int natural_notes[] = { 0, 2, 4, 5, 7, 9, 11, 12 };
	int octave_start_note = (int)(natural_number) / number_of_naturals_per_octave * number_of_notes_per_octave;
	int natural_number_in_octave = (int)(natural_number) % number_of_naturals_per_octave;
	int note_in_octave = natural_notes[natural_number_in_octave];
	int next_note_in_octave = natural_notes[natural_number_in_octave + 1];
	return octave_start_note + note_in_octave + ((next_note_in_octave - note_in_octave) * (natural_number - (int)(natural_number)));
}

float PianoWidget::getAccidentalNote(float accidental_number)
{
	return accidental_number; // might get fancier later
}

int main(int argc, char** argv)
{
	// qt bug workaround: otherwise some touch events get diverted for gesture recognition even with gestures disabled
	QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);

	QApplication application(argc, argv);
	application.setOrganizationName("Sreal");
	application.setOrganizationDomain("sreal.com");
	application.setApplicationName("TouchMIDI");

	midi_out = MidiOut::open(argv[1]);

	if (midi_out == NULL)
	{
		QMessageBox::warning(NULL, application.tr("Error"), application.tr("Cannot open the specified MIDI output port."));
		return 0;
	}

	QMainWindow* window = new QMainWindow();
	QWidget* panel = new QWidget();
	window->setCentralWidget(panel);
	QVBoxLayout* layout = new QVBoxLayout(panel);
	layout->addWidget(new PianoWidget(), 1);
	layout->addWidget(new PianoWidget(), 1);
	layout->addWidget(new PianoWidget(), 1);
	window->resize(640, 480);
	window->show();

	return application.exec();
}


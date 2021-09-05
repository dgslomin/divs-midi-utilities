
#include <QtWidgets>
#include <rtmidi_c.h>
#include "midiutil-common.h"
#include "midiutil-rtmidi.h"
#include "tactrola.h"

#define SETTLE_TIMER_RESOLUTION_MSECS 5
#define SETTLE_DELAY_MSECS 50
#define SETTLE_RATE_MSECS_PER_KEY 250

#define RED_TOP

#ifdef RED_TOP
#define TOP_COLOR QColor(242, 33, 10)
#define TOP_LINE_COLOR QColor(0, 0, 0, 60)
#define TOP_FILL_COLOR QColor(0, 0, 0, 20)
#define TOP_TEXT_COLOR QColor(255, 255, 255, 180)
#else
#define TOP_COLOR Qt::black
#define TOP_LINE_COLOR QColor(255, 255, 255, 100)
#define TOP_FILL_COLOR QColor(255, 255, 255, 60)
#define TOP_TEXT_COLOR QColor(255, 255, 255, 180)
#endif

#ifdef INVERTED_KEYS
#define BOTTOM_COLOR QColor(60, 60, 60)
#define NATURAL_COLOR Qt::black
#define ACCIDENTAL_COLOR Qt::white
#else
#define BOTTOM_COLOR QColor(230, 230, 230)
#define NATURAL_COLOR Qt::white
#define ACCIDENTAL_COLOR Qt::black
#endif

QVector<QString> MidiOut::getPortNames()
{
	QVector<QString> port_names;
	RtMidiOutPtr underlying_midi_out = rtmidi_out_create(RTMIDI_API_UNSPECIFIED, "Tactrola");
	int number_of_ports = rtmidi_get_port_count(underlying_midi_out);
	for (int port_number = 0; port_number < number_of_ports; port_number++) port_names.append(rtmidi_get_port_name(underlying_midi_out, port_number));
	rtmidi_close_port(underlying_midi_out);
	return port_names;
}

MidiOut* MidiOut::open(QString port_name)
{
	RtMidiOutPtr underlying_midi_out;
	if ((underlying_midi_out = rtmidi_open_out_port((char *)("Tactrola"), (char *)(port_name.toStdString().c_str()), (char *)("Tactrola"))) == NULL) return NULL;
	return new MidiOut(port_name, underlying_midi_out);
}

MidiOut::MidiOut(QString port_name, RtMidiOutPtr underlying_midi_out)
{
	this->port_name = port_name;
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
	this->channel_to_note[channel] = note + this->transpose;

	// reset pitch wheel value and range
	this->pitchWheel(channel, 1 << 13);
	this->controlChange(channel, 101, 0);
	this->controlChange(channel, 100, 0);
	this->controlChange(channel, 6, 48);
	this->controlChange(channel, 38, 0);

	this->noteOn(channel, note + this->transpose, this->velocity);
}

void MidiOut::mpePitchWheel(int finger_id, int amount)
{
	int channel = this->finger_id_to_channel.value(finger_id, -1);
	if (channel < 0) return;
	this->pitchWheel(channel, amount);
}

void MidiOut::mpeAllNotesOff()
{
	for (int finger_id: this->finger_id_to_channel.keys()) this->mpeNoteOff(finger_id);
}

TouchWidget::TouchWidget(Window* window)
{
	this->window = window;
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

PianoWidget::PianoWidget(Window* window): TouchWidget(window)
{
	this->startTimer(SETTLE_TIMER_RESOLUTION_MSECS);
}

void PianoWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	this->full_width = qMax(this->full_width, this->width());
	this->pan = qMax(qMin(this->pan, this->full_width - this->width()), 0);

	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), NATURAL_COLOR);

	int full_number_of_notes = 128;
	int number_of_notes_per_octave = 12;
	int full_number_of_naturals = 75;
	int number_of_naturals_per_octave = 7;
	bool note_is_accidental[] = { false, true, false, true, false, false, true, false, true, false, true, false };
	float natural_width = (float)(this->full_width) / full_number_of_naturals;
	float accidental_width = natural_width * number_of_naturals_per_octave / number_of_notes_per_octave;

	painter.setPen(ACCIDENTAL_COLOR);

	for (int natural_number = 1; natural_number < full_number_of_naturals; natural_number++)
	{
		float x = natural_number * natural_width - this->pan;
		painter.drawLine(x, 0, x, this->height());
	}

	for (int note_number = 0; note_number < full_number_of_notes; note_number++)
	{
		float x = note_number * accidental_width - this->pan;
		if (note_is_accidental[note_number % number_of_notes_per_octave]) painter.fillRect(x, 0, accidental_width, this->height() / 2, ACCIDENTAL_COLOR);
	}
}

void PianoWidget::touchEvent(QTouchEvent* event)
{
	event->accept();

	if (this->adjust_range)
	{
		switch (event->touchPoints().size())
		{
			case 1:
			{
				const QTouchEvent::TouchPoint& touch_point = event->touchPoints()[0];
				this->pan -= touch_point.pos().x() - touch_point.lastPos().x();
				this->update();
				break;
			}
			case 2:
			{
				// TODO: this works but behaves awkwardly
				const QTouchEvent::TouchPoint& touch_point_1 = event->touchPoints()[0];
				const QTouchEvent::TouchPoint& touch_point_2 = event->touchPoints()[1];
				this->full_width += (qAbs(touch_point_1.pos().x() - touch_point_2.pos().x()) - qAbs(touch_point_1.lastPos().x() - touch_point_2.lastPos().x()));
				this->update();
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else
	{
		for (const QTouchEvent::TouchPoint& touch_point: event->touchPoints())
		{
			int finger_id = touch_point.id();
			QPointF pos = touch_point.pos();

			switch (touch_point.state())
			{
				case Qt::TouchPointPressed:
				{
					float auto_aim_amount;
					int note = this->getStartNote(pos.x(), pos.y(), auto_aim_amount);
					this->window->midi_out->mpeNoteOn(finger_id, note);

					PianoWidget::FingerState finger_state;
					finger_state.x = pos.x();
					finger_state.y = pos.y();
					finger_state.start_y = pos.y();
					finger_state.start_note = note;
					finger_state.note = note;
					finger_state.auto_aim_amount = auto_aim_amount;
					finger_state.settle_duration.start();
					this->finger_states.insert(finger_id, finger_state);

					break;
				}
				case Qt::TouchPointMoved:
				{
					PianoWidget::FingerState finger_state = this->finger_states.value(finger_id);

					if (this->glide)
					{
						float note = this->getGlideNote(pos.x(), pos.y(), finger_state.start_y, finger_state.auto_aim_amount);

						if (note != finger_state.note)
						{
							int amount = this->getPitchWheelAmount(note - finger_state.start_note);
							this->window->midi_out->mpePitchWheel(finger_id, amount);
							finger_state.note = note;
						}
					}
					else
					{
						float auto_aim_amount;
						int note = this->getStartNote(pos.x(), pos.y(), auto_aim_amount);

						if (note != finger_state.note)
						{
							this->window->midi_out->mpeNoteOff(finger_id);
							this->window->midi_out->mpeNoteOn(finger_id, note);
							finger_state.start_note = note;
							finger_state.note = note;
						}

						finger_state.start_y = pos.y();
						finger_state.auto_aim_amount = auto_aim_amount;
					}

					finger_state.x = pos.x();
					finger_state.y = pos.y();
					finger_state.settle_duration.start();
					this->finger_states.insert(finger_id, finger_state);
					break;
				}
				case Qt::TouchPointReleased:
				{
					this->window->midi_out->mpeNoteOff(finger_id);
					this->finger_states.remove(finger_id);
					break;
				}
				default:
				{
					break;
				}
			}
		}
	}
}

void PianoWidget::timerEvent(QTimerEvent* event)
{
	Q_UNUSED(event)
	if (!this->glide) return;

	for (QHash<int, PianoWidget::FingerState>::iterator finger_state_iterator = this->finger_states.begin(); finger_state_iterator != this->finger_states.end(); finger_state_iterator++)
	{
		FingerState& finger_state = *finger_state_iterator;
		qint64 settle_duration_msecs = finger_state.settle_duration.elapsed();

		if (settle_duration_msecs > SETTLE_DELAY_MSECS)
		{
			float note = this->getSettleNote(finger_state.x, finger_state.y, finger_state.start_y, finger_state.auto_aim_amount);

			if (note != finger_state.note)
			{
				int amount = this->getPitchWheelAmount(note - finger_state.start_note);
				this->window->midi_out->mpePitchWheel(finger_state_iterator.key(), amount);
				finger_state.note = note;
			}
		}
	}
}

/** When you first press your finger, the note is instantly auto-aimed to be squarely in tune. */
int PianoWidget::getStartNote(int x, int y, float& auto_aim_amount)
{
	if (this->isNatural(y))
	{
		float natural_number = this->getNaturalNumber(x);
		int aimed_natural_number = (int)(natural_number);
		auto_aim_amount = aimed_natural_number - natural_number;
		return (int)(this->getNaturalNote(aimed_natural_number));
	}
	else
	{
		float accidental_number = this->getAccidentalNumber(x);
		int aimed_accidental_number = (int)(accidental_number);
		auto_aim_amount = aimed_accidental_number - accidental_number;
		return (int)(this->getAccidentalNote(aimed_accidental_number));
	}
}

/** When you glide your finger between keys, you get the microtonal "in-between" notes. */
float PianoWidget::getGlideNote(int x, int y, int start_y, float auto_aim_amount)
{
	float natural_number = this->getNaturalNumber(x) + auto_aim_amount;
	float natural_note = this->getNaturalNote(natural_number);

	float accidental_number = this->getAccidentalNumber(x) + auto_aim_amount;
	float accidental_note = this->getAccidentalNote(accidental_number);

	float accidental_fraction = this->getAccidentalFraction(y, start_y);
	return this->interpolate(natural_note, accidental_note, accidental_fraction);
}

/** When you stop moving your finger, it auto-aims back into tune gradually. */
float PianoWidget::getSettleNote(int x, int y, int start_y, float& auto_aim_amount)
{
	float natural_number = this->getNaturalNumber(x) + auto_aim_amount;
	float natural_auto_aim_increment = this->getSettleAutoAimIncrement(natural_number);

	float accidental_number = this->getAccidentalNumber(x) + auto_aim_amount;
	float accidental_auto_aim_increment = this->getSettleAutoAimIncrement(accidental_number);

	float accidental_fraction = this->getAccidentalFraction(y, start_y);
	float auto_aim_increment = this->interpolate(natural_auto_aim_increment, accidental_auto_aim_increment, accidental_fraction);
	auto_aim_amount += auto_aim_increment;

	float natural_note = this->getNaturalNote(natural_number - auto_aim_increment);
	float accidental_note = this->getAccidentalNote(accidental_number - auto_aim_increment);
	return this->interpolate(natural_note, accidental_note, accidental_fraction);
}

float PianoWidget::getSettleAutoAimIncrement(float key_number)
{
	int aimed_key_number = qRound(key_number);
	float full_auto_aim_increment = aimed_key_number - key_number;
	float max_auto_aim_increment = (float)(SETTLE_TIMER_RESOLUTION_MSECS) / SETTLE_RATE_MSECS_PER_KEY;

	if (full_auto_aim_increment > 0)
	{
		return qMin(full_auto_aim_increment, max_auto_aim_increment);
	}
	else
	{
		return qMax(full_auto_aim_increment, -max_auto_aim_increment);
	}
}

bool PianoWidget::isNatural(int y)
{
	return (y > this->height() / 2);
}

float PianoWidget::getAccidentalFraction(int y, int start_y)
{
	int key_height = this->height() / 2;

	if (this->isNatural(start_y))
	{
		return (float)(qMin(qMax(start_y - y, 0), key_height)) / key_height;
	}
	else
	{
		return 1.0 - ((float)(qMin(qMax(y - start_y, 0), key_height)) / key_height);
	}
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
	return octave_start_note + this->interpolate(note_in_octave, next_note_in_octave, natural_number - (int)(natural_number));
}

float PianoWidget::getAccidentalNote(float accidental_number)
{
	return accidental_number;
}

float PianoWidget::interpolate(float value1, float value2, float fraction)
{
	return value1 + ((value2 - value1) * fraction);
}

int PianoWidget::getPitchWheelAmount(float note_offset)
{
	int full_pitch_wheel_range = 1 << 14;
	int full_pitch_wheel_range_notes = 96;
	return (int)(note_offset * full_pitch_wheel_range / full_pitch_wheel_range_notes) + (1 << 13);
}

void PianoWidget::setAdjustRange(bool adjust_range)
{
	this->adjust_range = adjust_range;
	this->window->midi_out->mpeAllNotesOff();
}

void PianoWidget::setGlide(bool glide)
{
	this->glide = glide;
}

SliderWidget::SliderWidget(Window* window, QString label): TouchWidget(window)
{
	this->label = label;
}

void SliderWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), TOP_COLOR);

	painter.setPen(TOP_LINE_COLOR);
	painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

	painter.setPen(Qt::NoPen);
	painter.setBrush(TOP_FILL_COLOR);
	painter.drawRect(0, this->height() * (1.0 - this->value), this->width(), this->height() * this->value);

	painter.setPen(TOP_TEXT_COLOR);
	painter.setBrush(Qt::NoBrush);
	painter.drawText(0, 0, this->width(), this->height() - 15, Qt::AlignBottom | Qt::AlignHCenter, this->label);
}

void SliderWidget::touchEvent(QTouchEvent* event)
{
	for (const QTouchEvent::TouchPoint& touch_point: event->touchPoints())
	{
		switch (touch_point.state())
		{
			case Qt::TouchPointPressed:
			case Qt::TouchPointMoved:
			{
				int zone_leader_channel = 0;
				int max_controller_value = 127;
				float value_offset = (touch_point.pos().y() - touch_point.lastPos().y()) / this->height();
				this->value = qMin(qMax((this->value - value_offset), 0.0f), 1.0f);
				if (this->controller_number >= 0) this->window->midi_out->controlChange(zone_leader_channel, this->controller_number, (int)(max_controller_value * this->value));
				this->update();
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void SliderWidget::setControllerNumber(int controller_number)
{
	if (this->controller_number != controller_number)
	{
		this->controller_number = controller_number;
		this->value = 0;
		this->update();
	}
}

Button::Button(Window* window, QString label): TouchWidget(window)
{
	this->label = label;
}

void Button::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), TOP_COLOR);

	painter.setPen(TOP_LINE_COLOR);
	painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

	painter.setPen(TOP_TEXT_COLOR);
	painter.setBrush(Qt::NoBrush);
	painter.drawText(0, 0, this->width(), this->height() - 15, Qt::AlignBottom | Qt::AlignHCenter, this->label);
}

void Button::touchEvent(QTouchEvent* event)
{
	event->accept();
	if (event->touchPointStates() == Qt::TouchPointPressed) emit this->triggered();
}

ShiftButton::ShiftButton(Window* window, QString label): TouchWidget(window)
{
	this->label = label;
}

void ShiftButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), TOP_COLOR);

	painter.setPen(TOP_LINE_COLOR);
	painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

	painter.setPen(TOP_TEXT_COLOR);
	painter.setBrush(Qt::NoBrush);
	painter.drawText(0, 0, this->width(), this->height() - 15, Qt::AlignBottom | Qt::AlignHCenter, this->label);
}

void ShiftButton::touchEvent(QTouchEvent* event)
{
	event->accept();
	bool is_pressed = (event->touchPointStates() != Qt::TouchPointReleased);

	if (is_pressed != this->is_pressed)
	{
		this->is_pressed = is_pressed;
		emit this->stateChanged(is_pressed);
	}
}

LatchButton::LatchButton(Window* window, QString label): TouchWidget(window)
{
	this->label = label;
}

void LatchButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), TOP_COLOR);

	painter.setPen(TOP_LINE_COLOR);
	if (this->is_pressed) painter.setBrush(TOP_FILL_COLOR);
	painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

	painter.setPen(TOP_TEXT_COLOR);
	painter.setBrush(Qt::NoBrush);
	painter.drawText(0, 0, this->width(), this->height() - 15, Qt::AlignBottom | Qt::AlignHCenter, this->label);
}

void LatchButton::touchEvent(QTouchEvent* event)
{
	event->accept();

	if (event->touchPointStates() == Qt::TouchPointPressed)
	{
		this->is_pressed = !(this->is_pressed);
		emit this->stateChanged(this->is_pressed);
		this->update();
	}
}

void LatchButton::setPressed(bool is_pressed)
{
	if (is_pressed != this->is_pressed)
	{
		this->is_pressed = is_pressed;
		emit this->stateChanged(is_pressed);
		this->update();
	}
}

Window::Window()
{
	QWidget* panel = new QWidget();
	this->setCentralWidget(panel);
	panel->setAutoFillBackground(true);
	panel->setPalette(QPalette(BOTTOM_COLOR));
	QVBoxLayout* layout = new QVBoxLayout(panel);
	layout->setContentsMargins(0, 0, 0, 0);

	QWidget* top_row_panel = new QWidget();
	layout->addWidget(top_row_panel, 1);
	top_row_panel->setAutoFillBackground(true);
	top_row_panel->setPalette(QPalette(TOP_COLOR));
	QHBoxLayout* top_row_layout = new QHBoxLayout(top_row_panel);
	top_row_layout->setContentsMargins(15, 15, 15, 15);
	top_row_layout->setSpacing(15);

	for (int slider_number = 0; slider_number < 16; slider_number++)
	{
		SliderWidget* slider = new SliderWidget(this, QString::number(slider_number + 1));
		this->sliders[slider_number] = slider;
		top_row_layout->addWidget(slider, 1);
	}

	QWidget* buttons_panel = new QWidget();
	top_row_layout->addWidget(buttons_panel, 1);
	QVBoxLayout* buttons_layout = new QVBoxLayout(buttons_panel);
	buttons_layout->setContentsMargins(0, 0, 0, 0);
	buttons_layout->setSpacing(15);

	this->setup_button = new Button(this, tr("Setup"));
	buttons_layout->addWidget(this->setup_button, 1);

	this->range_button = new ShiftButton(this, tr("Range"));
	buttons_layout->addWidget(this->range_button, 1);

	this->glide_button = new LatchButton(this, tr("Glide"));
	buttons_layout->addWidget(this->glide_button, 1);

	this->upper_keyboard = new PianoWidget(this);
	layout->addWidget(this->upper_keyboard, 1);

	this->lower_keyboard = new PianoWidget(this);
	layout->addWidget(this->lower_keyboard, 1);

	this->resize(640, 480);

	this->connect(this->setup_button, SIGNAL(triggered()), this, SLOT(showSetupDialog()));
	this->connect(this->range_button, SIGNAL(stateChanged(bool)), this->upper_keyboard, SLOT(setAdjustRange(bool)));
	this->connect(this->range_button, SIGNAL(stateChanged(bool)), this->lower_keyboard, SLOT(setAdjustRange(bool)));
	this->connect(this->glide_button, SIGNAL(stateChanged(bool)), this->upper_keyboard, SLOT(setGlide(bool)));
	this->connect(this->glide_button, SIGNAL(stateChanged(bool)), this->lower_keyboard, SLOT(setGlide(bool)));

	QAction* toggle_fullscreen_action = new QAction(tr("Fullscreen"));
	this->addAction(toggle_fullscreen_action);
	toggle_fullscreen_action->setShortcut(QKeySequence(QKeySequence::FullScreen));
	connect(toggle_fullscreen_action, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));

	QSettings settings;
	this->restoreGeometry(settings.value("window-geometry").toByteArray());
	this->restoreState(settings.value("window-state").toByteArray());
	this->setMidiOut(settings.value("midi-output-port-name", "").toString());
	this->midi_out->velocity = settings.value("velocity", 127).toInt();
	this->midi_out->transpose = settings.value("transpose", 0).toInt();

	for (int slider_number = 0; slider_number < 16; slider_number++)
	{
		SliderWidget* slider = this->sliders[slider_number];
		slider->controller_number = settings.value(QString("slider-%1-controller-number").arg(slider_number + 1), -1).toInt();
		slider->value = settings.value(QString("slider-%1-value").arg(slider_number + 1), 0.0).toFloat();
	}

	this->glide_button->setPressed(settings.value("glide", false).toBool());
	this->upper_keyboard->full_width = settings.value("upper-keyboard-full-width", INT_MIN).toInt();
	this->upper_keyboard->pan = settings.value("upper-keyboard-pan", INT_MAX).toInt();
	this->lower_keyboard->full_width = settings.value("lower-keyboard-full-width", INT_MIN).toInt();
	this->lower_keyboard->pan = settings.value("lower-keyboard-pan", INT_MAX).toInt();
}

void Window::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event)
	QSettings settings;
	settings.setValue("window-geometry", this->saveGeometry());
	settings.setValue("window-state", this->saveState());
	settings.setValue("midi-output-port-name", this->midi_out->port_name);
	settings.setValue("velocity", this->midi_out->velocity);
	settings.setValue("transpose", this->midi_out->transpose);

	for (int slider_number = 0; slider_number < 16; slider_number++)
	{
		SliderWidget* slider = this->sliders[slider_number];
		settings.setValue(QString("slider-%1-controller-number").arg(slider_number + 1), slider->controller_number);
		settings.setValue(QString("slider-%1-value").arg(slider_number + 1), slider->value);
	}

	settings.setValue("glide", this->glide_button->is_pressed);
	settings.setValue("upper-keyboard-full-width", this->upper_keyboard->full_width);
	settings.setValue("upper-keyboard-pan", this->upper_keyboard->pan);
	settings.setValue("lower-keyboard-full-width", this->lower_keyboard->full_width);
	settings.setValue("lower-keyboard-pan", this->lower_keyboard->pan);
}

void Window::setMidiOut(QString port_name)
{
	if (this->midi_out == NULL)
	{
		this->midi_out = MidiOut::open(port_name);
	}
	else
	{
		if (this->midi_out->port_name != port_name)
		{
			delete(this->midi_out);
			this->midi_out = MidiOut::open(port_name);
		}
	}
}

void Window::showSetupDialog()
{
	SetupDialog setup_dialog(this);

	if (setup_dialog.exec() == QDialog::Accepted)
	{
		this->setMidiOut(setup_dialog.midi_output_port_combo_box->currentText());
		this->midi_out->velocity = setup_dialog.velocity_spin_box->value();
		this->midi_out->transpose = setup_dialog.transpose_spin_box->value();

		for (int slider_number = 0; slider_number < 16; slider_number++)
		{
			this->sliders[slider_number]->setControllerNumber(setup_dialog.slider_controller_number_combo_boxes[slider_number]->currentData().toInt());
		}
	}
}

void Window::toggleFullscreen()
{
	if (this->isFullScreen())
	{
		this->showNormal();
	}
	else
	{
		this->showFullScreen();
	}
}

SetupDialog::SetupDialog(Window* window)
{
	this->window = window;
	this->setWindowTitle(tr("Setup"));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(15);

	QFormLayout* general_settings_layout = new QFormLayout();
	layout->addLayout(general_settings_layout);

	this->midi_output_port_combo_box = new QComboBox();
	general_settings_layout->addRow(tr("MIDI Output Port"), this->midi_output_port_combo_box);

	for (QString port_name: MidiOut::getPortNames())
	{
		this->midi_output_port_combo_box->addItem(port_name);
		if ((this->window->midi_out != NULL) && (this->window->midi_out->port_name == port_name)) this->midi_output_port_combo_box->setCurrentIndex(this->midi_output_port_combo_box->count() - 1);
	}

	this->velocity_spin_box = new QSpinBox();
	general_settings_layout->addRow(tr("Velocity"), this->velocity_spin_box);
	this->velocity_spin_box->setRange(0, 127);
	this->velocity_spin_box->setValue(this->window->midi_out->velocity);

	this->transpose_spin_box = new QSpinBox();
	general_settings_layout->addRow(tr("Transpose"), this->transpose_spin_box);
	this->transpose_spin_box->setRange(-127, 127);
	this->transpose_spin_box->setValue(this->window->midi_out->transpose);

	QGroupBox* slider_controllers_group_box = new QGroupBox(tr("Slider Controllers"));
	layout->addWidget(slider_controllers_group_box);

	QHBoxLayout* two_column_layout = new QHBoxLayout(slider_controllers_group_box);
	QFormLayout* left_column_layout = new QFormLayout();
	two_column_layout->addLayout(left_column_layout);
	QFormLayout* right_column_layout = new QFormLayout();
	two_column_layout->addLayout(right_column_layout);

	for (int slider_number = 0; slider_number < 16; slider_number++)
	{
		QComboBox* controller_number_combo_box = new QComboBox();
		((slider_number < 8) ? left_column_layout : right_column_layout)->addRow(QString::number(slider_number + 1), controller_number_combo_box);
		controller_number_combo_box->addItem(tr("None"), -1);

		for (int controller_number = 0; controller_number < 128; controller_number++)
		{
			QString label;

			switch (controller_number)
			{
				case 1:
				{
					label = tr("Modulation wheel");
					break;
				}
				case 2:
				{
					label = tr("Breath controller");
					break;
				}
				case 5:
				{
					label = tr("Portamento time");
					break;
				}
				case 7:
				{
					label = tr("Volume");
					break;
				}
				case 8:
				{
					label = tr("Balance");
					break;
				}
				case 9:
				{
					label = tr("Pan");
					break;
				}
				case 11:
				{
					label = tr("Expression");
					break;
				}
				case 64:
				{
					label = tr("Sustain pedal");
					break;
				}
				case 65:
				{
					label = tr("Portamento switch");
					break;
				}
				case 71:
				{
					label = tr("Resonance");
					break;
				}
				case 72:
				{
					label = tr("Release");
					break;
				}
				case 73:
				{
					label = tr("Attack");
					break;
				}
				case 74:
				{
					label = tr("Cutoff");
					break;
				}
				case 91:
				{
					label = tr("Reverb");
					break;
				}
				case 92:
				{
					label = tr("Tremolo");
					break;
				}
				case 93:
				{
					label = tr("Chorus");
					break;
				}
				case 95:
				{
					label = tr("Phaser");
					break;
				}
				default:
				{
					break;
				}
			}

			controller_number_combo_box->addItem(QString("%1%2").arg(controller_number).arg(label.isEmpty() ? "" : QString(" - %1").arg(label)), controller_number);
			if (this->window->sliders[slider_number]->controller_number == controller_number) controller_number_combo_box->setCurrentIndex(controller_number_combo_box->count() - 1);
		}

		this->slider_controller_number_combo_boxes[slider_number] = controller_number_combo_box;
	}

	QDialogButtonBox* dialog_button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	layout->addWidget(dialog_button_box);

	connect(dialog_button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(dialog_button_box, SIGNAL(rejected()), this, SLOT(reject()));
}

int main(int argc, char** argv)
{
	// qt bug workaround: otherwise some touch events get diverted for gesture recognition even with gestures disabled
	QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);

	QApplication application(argc, argv);
	application.setOrganizationName("Sreal");
	application.setOrganizationDomain("sreal.com");
	application.setApplicationName("Tactrola");
	(new Window())->show();
	return application.exec();
}


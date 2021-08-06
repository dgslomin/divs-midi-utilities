
#include <QtWidgets>
#include <rtmidi_c.h>
#include "midiutil-rtmidi.h"
#include "touchmidi.h"

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
	// skip channel 0 since it's the zone leader
	for (int channel = 1; channel < 16; channel++) this->available_channels.append(channel);
}

void PianoWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	this->full_width = qMax(this->full_width, this->width());
	this->pan = qMin(this->pan, this->full_width - this->width());

	QPainter painter(this);
	painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
	painter.setPen(Qt::black);

	int number_of_naturals_per_octave = 7;
	bool natural_has_a_flat[] = { false, true, true, false, true, true, true };
	int full_number_of_naturals = 75;
	float natural_width = (float)(this->full_width) / full_number_of_naturals;

	for (int natural_number = 1; natural_number < full_number_of_naturals; natural_number++)
	{
		float x = natural_number * natural_width - this->pan;
		painter.drawLine(x, 0, x, this->height());

		if (natural_has_a_flat[natural_number % number_of_naturals_per_octave])
		{
			painter.fillRect(x - (natural_width / 4), 0, natural_width / 2, this->height() / 2, Qt::black);
		}
	}
}

void PianoWidget::touchEvent(QTouchEvent* event)
{
	for (const QTouchEvent::TouchPoint& touch_point: event->touchPoints())
	{
		switch (touch_point.state())
		{
			case Qt::TouchPointPressed:
			{
				QPointF pos = touch_point.pos();
				int note = this->getNoteForXY(pos.x(), pos.y());
				if (note < 0) continue;
				this->noteOn(touch_point.id(), note);
				break;
			}
			case Qt::TouchPointMoved:
			{
				int amount = this->getBendAmountForXOffset(touch_point.pos().x() - touch_point.startPos().x());
				this->bend(touch_point.id(), amount);
				break;
			}
			case Qt::TouchPointReleased:
			{
				this->noteOff(touch_point.id());
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

int PianoWidget::getNoteForXY(int x, int y)
{
	int number_of_naturals_per_octave = 7;
	int number_of_notes_per_octave = 12;
	int natural_note[] = { 0, 2, 4, 5, 7, 9, 11 };
	int sharp_note[] = { 1, 3, -1, 6, 8, 9, -1 };
	int full_number_of_naturals = 75;
	float natural_width = (float)(this->full_width) / full_number_of_naturals;

	if (y > this->height() / 2)
	{
		int natural_number = (x + this->pan) / natural_width;
		int note_in_octave = natural_note[natural_number % number_of_naturals_per_octave];
		int octave_number = natural_number / number_of_naturals_per_octave;
		int octave_start_note = octave_number * number_of_notes_per_octave;
		return octave_start_note + note_in_octave;
	}
	else
	{
		int natural_number = (x + this->pan - (natural_width / 2)) / natural_width;
		int note_in_octave = sharp_note[natural_number % number_of_naturals_per_octave];
		if (note_in_octave < 0) return -1;
		int octave_number = natural_number / number_of_naturals_per_octave;
		int octave_start_note = octave_number * number_of_notes_per_octave;
		return octave_start_note + note_in_octave;
	}

	return 0;
}

int PianoWidget::getBendAmountForXOffset(int x_offset)
{
	int full_number_of_notes = 128;
	float note_width = (float)(this->full_width) / full_number_of_notes;
	float note_offset = x_offset / note_width;
	int full_bend_range = 2 << 14;
	int full_bend_range_notes = 96;
	return (int)(note_offset * full_bend_range / full_bend_range_notes) + (2 << 13);
}

void PianoWidget::noteOn(int finger_id, int note)
{
	// This is an MPE channel allocation algorithm that forces a single note per
	// channel and kicks out the oldest note if there's no channel available for
	// a new note.

	int channel;

	if (this->available_channels.empty())
	{
		channel = this->busy_channels.takeFirst();
		this->finger_id_to_channel.remove(this->channel_to_finger_id[channel]);
		qDebug("noteOff(channel: %d, note: %d)", channel, this->channel_to_note[channel]);
	}
	else
	{
		channel = this->available_channels.takeFirst();
	}

	this->busy_channels.append(channel);
	this->finger_id_to_channel.insert(finger_id, channel);
	this->channel_to_finger_id[channel] = finger_id;
	this->channel_to_note[channel] = note;
	qDebug("noteOn(channel: %d, note: %d)", channel, note);
}

void PianoWidget::bend(int finger_id, int amount)
{
	int channel = this->finger_id_to_channel.value(finger_id, -1);
	if (channel < 0) return;
	qDebug("bend(channel: %d, amount: %d)", channel, amount);
}

void PianoWidget::noteOff(int finger_id)
{
	int channel = this->finger_id_to_channel.value(finger_id, -1);
	if (channel < 0) return;
	int note = this->channel_to_note[channel];
	this->finger_id_to_channel.remove(finger_id);
	this->busy_channels.removeOne(channel);
	this->available_channels.append(channel);
	qDebug("noteOff(channel: %d, note: %d)", channel, note);
}

int main(int argc, char** argv)
{
	QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

	QApplication application(argc, argv);
	application.setOrganizationName("Sreal");
	application.setOrganizationDomain("sreal.com");
	application.setApplicationName("TouchMIDI");

	QMainWindow* window = new QMainWindow();
	QWidget* panel = new QWidget();
	window->setCentralWidget(panel);
	QVBoxLayout* layout = new QVBoxLayout(panel);

	PianoWidget* upper = new PianoWidget();
	layout->addWidget(upper, 1);

	PianoWidget* lower = new PianoWidget();
	layout->addWidget(lower, 1);

	window->resize(640, 480);
	window->show();

	return application.exec();
}


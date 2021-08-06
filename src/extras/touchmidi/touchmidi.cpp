
#include <QtWidgets>
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
	for (int touch_point_number = 0; touch_point_number < event->touchPoints().size(); touch_point_number++)
	{
		QTouchEvent::TouchPoint touch_point = event->touchPoints()[touch_point_number];
		QString state;

		switch (touch_point.state())
		{
			case Qt::TouchPointPressed:
			{
				state = "Pressed";
				break;
			}
			case Qt::TouchPointMoved:
			{
				state = "Moved";
				break;
			}
			case Qt::TouchPointStationary:
			{
				state = "Stationary";
				break;
			}
			case Qt::TouchPointReleased:
			{
				state = "Released";
				break;
			}
			default:
			{
				state = "???";
				break;
			}
		}

		QPointF start_pos = touch_point.startPos();
		QPointF pos = touch_point.pos();
		qDebug("touch %d: start(%f, %f), current(%f, %f) -> %s", touch_point_number, start_pos.x(), start_pos.y(), pos.x(), pos.y(), state.toStdString().c_str());
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


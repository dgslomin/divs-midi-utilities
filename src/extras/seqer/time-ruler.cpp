
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QSettings>
#include <QString>
#include <QWheelEvent>
#include "colors.h"
#include "midifile.h"
#include "time-ruler.h"
#include "window.h"

TimeRuler::TimeRuler(Window* window)
{
	this->window = window;

	QSettings settings;
	float font_scaling_factor = settings.value("time-ruler/font-scaling-factor", 0.8).toFloat();

	QFont font = this->font();
	font.setPointSizeF(font.pointSizeF() * font_scaling_factor);
	this->setFont(font);

	this->setFixedHeight(this->fontMetrics().lineSpacing() * 2);
}

void TimeRuler::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)

	QPainter painter(this);
	QFontMetrics font_metrics = painter.fontMetrics();
	int hour_minute_second_y = font_metrics.leading() + font_metrics.ascent();
	int measure_beat_y = font_metrics.lineSpacing() + font_metrics.ascent();

	painter.setPen(Colors::textShade(150, 200));

	{
		MidiFileHourMinuteSecond_t hour_minute_second = MidiFileHourMinuteSecond_new();
		int time = 0;
		int max_x = 0;

		while (true)
		{
			long tick = MidiFile_getTickFromTime(this->window->sequence->midi_file, time);
			int x = this->window->getXFromTick(tick);
			if (x >= this->width()) break;
			MidiFile_setHourMinuteSecondFromTick(this->window->sequence->midi_file, tick, hour_minute_second);

			QString label = QString("%1:%2:%3   ")
				.arg(MidiFileHourMinuteSecond_getHour(hour_minute_second))
				.arg(MidiFileHourMinuteSecond_getMinute(hour_minute_second), 2, 10, QLatin1Char('0'))
				.arg((int)(MidiFileHourMinuteSecond_getSecond(hour_minute_second)), 2, 10, QLatin1Char('0'));

			if ((tick == 0) || (x > max_x))
			{
				painter.drawText(x, hour_minute_second_y, label);
				max_x = x + font_metrics.boundingRect(label).width();
			}

			time++;
		}

		MidiFileHourMinuteSecond_free(hour_minute_second);
	}

	{
		MidiFileMeasureBeat_t measure_beat = MidiFileMeasureBeat_new();
		int beat = 0;
		int max_x = 0;

		while (true)
		{
			long tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, beat);
			int x = this->window->getXFromTick(tick);
			if (x >= this->width()) break;
			MidiFile_setMeasureBeatFromTick(this->window->sequence->midi_file, tick, measure_beat);

			QString label = QString("%1:%2   ")
				.arg(MidiFileMeasureBeat_getMeasure(measure_beat))
				.arg((int)(MidiFileMeasureBeat_getBeat(measure_beat)), 2, 10, QLatin1Char('0'));

			if ((tick == 0) || (x > max_x))
			{
				painter.drawText(x, measure_beat_y, label);
				max_x = x + font_metrics.boundingRect(label).width();
			}

			beat++;
		}

		MidiFileMeasureBeat_free(measure_beat);
	}
}

void TimeRuler::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->window->cursor_x = event->pos().x();
		this->window->update();
	}
}

void TimeRuler::wheelEvent(QWheelEvent* event)
{
	int x_offset = event->angleDelta().x();
	bool control = ((event->modifiers() & Qt::ControlModifier) != 0);

	if (control)
	{
		this->window->zoomXBy(x_offset / 960.0 + 1.0);
	}
	else
	{
		this->window->scrollXBy(x_offset);
	}

	this->window->update();
}


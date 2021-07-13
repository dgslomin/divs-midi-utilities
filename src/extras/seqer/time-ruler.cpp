
#include <QFont>
#include <QGuiApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSettings>
#include <QString>
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

	this->setMinimumHeight(this->fontMetrics().lineSpacing() * 2);
}

void TimeRuler::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	long min_tick = this->window->getTickFromX(0);
	long max_tick = this->window->getTickFromX(this->width());
	int min_beat = std::max((int)(MidiFile_getBeatFromTick(this->window->sequence->midi_file, min_tick)) - 1, 0);
	int max_beat = (int)(MidiFile_getBeatFromTick(this->window->sequence->midi_file, max_tick)) + 1;
	int min_time = std::max((int)(MidiFile_getTimeFromTick(this->window->sequence->midi_file, min_tick)) - 1, 0);
	int max_time = (int)(MidiFile_getTimeFromTick(this->window->sequence->midi_file, max_tick)) + 1;
	int line_height = painter.fontMetrics().lineSpacing();
	int measure_beat_y;
	int hour_minute_second_y;
	int max_x;

	if (this->window->use_linear_time)
	{
		measure_beat_y = 0;
		hour_minute_second_y = line_height;
	}
	else
	{
		measure_beat_y = line_height;
		hour_minute_second_y = 0;
	}

	max_x = -this->width();

	for (int beat = min_beat; beat < max_beat; beat++)
	{
		long tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, beat);
		int x = this->window->getXFromTick(tick);
		QString label = QString("%1  ").arg(MidiFile_getMeasureBeatStringFromTick(this->window->sequence->midi_file, tick));

		if (x >= max_x)
		{
			painter.drawText(x, measure_beat_y + line_height - 2, label);
			max_x = x + painter.fontMetrics().horizontalAdvance(label);
		}
	}

	max_x = -this->width();

	for (int time = min_time; time < max_time; time++)
	{
		long tick = MidiFile_getTickFromTime(this->window->sequence->midi_file, time);
		int x = this->window->getXFromTick(tick);
		QString label = QString("%1  ").arg(MidiFile_getHourMinuteSecondStringFromTick(this->window->sequence->midi_file, tick));

		if (x >= max_x)
		{
			painter.drawText(x, hour_minute_second_y + line_height - 2, label);
			max_x = x + painter.fontMetrics().horizontalAdvance(label);
		}
	}
}


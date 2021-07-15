
#include <QFont>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QSettings>
#include <QString>
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
	QPainter painter(this);
	QFontMetrics font_metrics = painter.fontMetrics();
	int measure_beat_y;
	int hour_minute_second_y;

	if (this->window->use_linear_time)
	{
		measure_beat_y = font_metrics.leading() + font_metrics.ascent();
		hour_minute_second_y = font_metrics.lineSpacing() + font_metrics.ascent();
	}
	else
	{
		measure_beat_y = font_metrics.lineSpacing() + font_metrics.ascent();
		hour_minute_second_y = font_metrics.leading() + font_metrics.ascent();
	}

	painter.setPen(Colors::textShade(150, 200));

	{
		int beat = 0;
		int max_x = 0;

		while (true)
		{
			long tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, beat);
			int x = this->window->getXFromTick(tick);
			if (x >= this->width()) break;
			QString label = QString("%1   ").arg(MidiFile_getMeasureBeatStringFromTick(this->window->sequence->midi_file, tick));

			if ((tick == 0) || (x > max_x))
			{
				painter.drawText(x, measure_beat_y, label);
				max_x = x + font_metrics.boundingRect(label).width();
			}

			beat++;
		}
	}

	{
		int time = 0;
		int max_x = 0;

		while (true)
		{
			long tick = MidiFile_getTickFromTime(this->window->sequence->midi_file, time);
			int x = this->window->getXFromTick(tick);
			if (x >= this->width()) break;
			QString label = QString("%1   ").arg(MidiFile_getHourMinuteSecondStringFromTick(this->window->sequence->midi_file, tick));

			if ((tick == 0) || (x > max_x))
			{
				painter.drawText(x, hour_minute_second_y, label);
				max_x = x + font_metrics.boundingRect(label).width();
			}

			time++;
		}
	}
}


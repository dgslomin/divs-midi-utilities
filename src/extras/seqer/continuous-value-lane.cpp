
#include <QtWidgets>
#include "colors.h"
#include "continuous-value-lane.h"
#include "lane.h"
#include "midifile.h"
#include "window.h"

ContinuousValueLane::ContinuousValueLane(Window* window, QString type): Lane(window, type)
{
	QSettings settings;
	this->handle_size = settings.value("continuous-value-lane/handle-size", 6).toInt();
	this->connecting_line_pen = QPen(settings.value("continuous-value-lane/connecting-line-color", Colors::buttonShade(200, 80)).value<QColor>());
}

void ContinuousValueLane::paintBackground(QPainter* painter)
{
	painter->fillRect(0, 0, this->width(), this->height(), this->background_color);
}

void ContinuousValueLane::paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset)
{
	QRect bounds(0, 0, this->width(), this->height());
	MidiFileTrack_t current_track = MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 0);

	painter->setPen(this->connecting_line_pen);
	QPoint last_point;

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->shouldIncludeEvent(midi_event) && (MidiFileEvent_getTrack(midi_event) == current_track))
		{
			QPoint point = this->getRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset).center();

			if (!last_point.isNull())
			{
				painter->drawLine(last_point.x(), last_point.y(), point.x(), last_point.y());
				painter->drawLine(point.x(), last_point.y(), point.x(), point.y());
			}

			last_point = point;
		}
	}

	if (!last_point.isNull() && (last_point.x() < this->width()))
	{
		painter->drawLine(last_point.x(), last_point.y(), this->width(), last_point.y());
	}

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->shouldIncludeEvent(midi_event))
		{
			QRect rect = this->getRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset);

			if (rect.intersects(bounds))
			{
				bool is_background = (MidiFileEvent_getTrack(midi_event) != current_track);
				bool is_selected = MidiFileEvent_isSelected(midi_event);
				painter->setPen(is_background ? (is_selected ? this->selected_background_event_pen : this->unselected_background_event_pen) : (is_selected ? this->selected_event_pen : this->unselected_event_pen));
				painter->setBrush(is_background ? (is_selected ? this->selected_background_event_brush : this->unselected_background_event_brush) : (is_selected ? this->selected_event_brush : this->unselected_event_brush));
				painter->drawRect(rect);
			}
		}
	}
}

MidiFileEvent_t ContinuousValueLane::getEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (this->shouldIncludeEvent(midi_event))
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);
			if (rect.contains(x, y, false)) return midi_event;
		}
	}

	return NULL;
}

QPoint ContinuousValueLane::getPointFromEvent(MidiFileEvent_t midi_event)
{
	int x = this->window->getXFromTick(MidiFileEvent_getTick(midi_event));
	int y = this->getYFromValue(this->getEventValue(midi_event));
	return QPoint(x, y);
}

MidiFileEvent_t ContinuousValueLane::addEventAtXY(int x, int y)
{
	return this->addEvent(this->window->getTickFromX(x), this->getValueFromY(y));
}

void ContinuousValueLane::moveEventsByXY(int x_offset, int y_offset)
{
	for (MidiFileEvent_t midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file))
	{
		if (this->shouldIncludeEvent(midi_event) && MidiFileEvent_isSelected(midi_event))
		{
			if (x_offset != 0) MidiFileEvent_setTick(midi_event, this->window->getTickFromX(this->window->getXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
			if (y_offset != 0) this->setEventValue(midi_event, this->getValueFromY(this->getYFromValue(this->getEventValue(midi_event)) + y_offset));
		}
	}
}

void ContinuousValueLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, y, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->shouldIncludeEvent(midi_event))
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);
			if (rect.intersects(bounds)) MidiFileEvent_setSelected(midi_event, 1);
		}
	}
}

void ContinuousValueLane::scrollYBy(int offset)
{
	this->scroll_y += offset;
	this->cursor_y += offset;
	this->update();
}

void ContinuousValueLane::zoomYBy(float factor)
{
	float cursor_value = this->getValueFromY(this->cursor_y);
	this->pixels_per_value *= factor;
	this->cursor_y = this->getYFromValue(cursor_value);
	this->update();
}

int ContinuousValueLane::getCursorGap()
{
	return this->handle_size;
}

QRect ContinuousValueLane::getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset)
{
	int x = this->window->getXFromTick(MidiFileEvent_getTick(midi_event));
	int y = this->getYFromValue(this->getEventValue(midi_event));

	if (MidiFileEvent_isSelected(midi_event))
	{
		x += selected_events_x_offset;
		y += selected_events_y_offset;
	}

	return QRect(x, y, this->handle_size, this->handle_size);
}

int ContinuousValueLane::getYFromValue(float value)
{
	return this->height() - this->handle_size - (int)(value * this->pixels_per_value) + this->scroll_y;
}

float ContinuousValueLane::getValueFromY(int y)
{
	return (float)(this->height() - this->handle_size - y + this->scroll_y) / this->pixels_per_value;
}


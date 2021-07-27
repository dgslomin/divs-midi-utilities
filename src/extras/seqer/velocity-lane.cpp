
#include <QtWidgets>
#include "lane.h"
#include "midifile.h"
#include "velocity-lane.h"
#include "window.h"

VelocityLane::VelocityLane(Window* window): Lane(window, Lane::VELOCITY_LANE_TYPE)
{
	QSettings settings;
	this->box_width = settings.value("velocity-lane/box-width", 6).toInt();
}

void VelocityLane::paintBackground(QPainter* painter)
{
	painter->fillRect(0, 0, this->width(), this->height(), this->background_color);
}

void VelocityLane::paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset)
{
	QRect bounds(0, 0, this->width(), this->height());
	MidiFileTrack_t current_track = MidiFile_getTrackByNumber(this->window->sequence->midi_file, this->track_number, 0);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE)
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

MidiFileEvent_t VelocityLane::getEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE)
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);
			if (rect.contains(x, y, false)) return midi_event;
		}
	}

	return NULL;
}

QPoint VelocityLane::getPointFromEvent(MidiFileEvent_t midi_event)
{
	int x = this->window->getXFromTick(MidiFileEvent_getTick(midi_event));
	int y = this->getYFromValue(MidiFileNoteEvent_getVelocity(midi_event));
	return QPoint(x, y);
}

MidiFileEvent_t VelocityLane::addEventAtXY(int x, int y)
{
	// not supported
	Q_UNUSED(x)
	Q_UNUSED(y)
	return NULL;
}

void VelocityLane::moveEventsByXY(int x_offset, int y_offset)
{
	for (MidiFileEvent_t midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file))
	{
		if ((MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE) && MidiFileEvent_isSelected(midi_event))
		{
			if (x_offset != 0) MidiFileEvent_setTick(midi_event, this->window->getTickFromX(this->window->getXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
			if (y_offset != 0) MidiFileNoteEvent_setVelocity(midi_event, (int)(this->getValueFromY(this->getYFromValue((MidiFileNoteEvent_getVelocity(midi_event) + y_offset)))));
		}
	}
}

void VelocityLane::selectEventsInRect(int x, int y, int width, int height)
{
	QRect bounds(x, y, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_NOTE)
		{
			QRect rect = this->getRectFromEvent(midi_event, 0, 0);
			if (rect.intersects(bounds)) MidiFileEvent_setSelected(midi_event, 1);
		}
	}
}

void VelocityLane::scrollYBy(int offset)
{
	this->scroll_y += offset;
	this->cursor_y += offset;
	this->update();
}

void VelocityLane::zoomYBy(float factor)
{
	float cursor_value = this->getValueFromY(this->cursor_y);
	this->pixels_per_value *= factor;
	this->cursor_y = this->getYFromValue(cursor_value);
	this->update();
}

int VelocityLane::getCursorGap()
{
	return 0;
}

QRect VelocityLane::getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset)
{
	int x = this->window->getXFromTick(MidiFileEvent_getTick(midi_event));
	int y = this->getYFromValue(MidiFileNoteEvent_getVelocity(midi_event));

	if (MidiFileEvent_isSelected(midi_event))
	{
		x += selected_events_x_offset;
		y += selected_events_y_offset;
	}

	return QRect(x, y, this->box_width, this->getYFromValue(0) - y);
}

int VelocityLane::getYFromValue(float value)
{
	return this->height() - (int)(value * this->pixels_per_value) + this->scroll_y;
}

float VelocityLane::getValueFromY(int y)
{
	return (float)(this->height() - y + this->scroll_y) / this->pixels_per_value;
}


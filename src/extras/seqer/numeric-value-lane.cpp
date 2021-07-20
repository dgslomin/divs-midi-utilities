
#include <QPainter>
#include "window.h"

NumericValueLane::NumericValueLane(Window* window): Lane(window)
{
}

void NumericValueLane::paintBackground(QPainter* painter)
{
	painter->fillRect(0, 0, this->width(), this->height(), this->background_color);
}

void NumericValueLane::paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset)
{
	QRect bounds(0, 0, this->width(), this->height());

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->shouldIncludeEvent(midi_event))
		{
			wxRect rect = this->getRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset);

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

MidiFileEvent_t NumericValueLane::getEventFromXY(int x, int y)
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

MidiFileEvent_t numericValueLane::addEventAtXY(int x, int y)
{
	return this->addEvent(this->window->getTickFromX(x), this->getValueFromY(y));
}

void NumericValueLane::moveEventsByXY(int x_offset, int y_offset)
{
	MidiFileEvent_setTick(midi_event, this->window->getTickFromX(this->window->GetXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
	this->SetEventValue(midi_event, this->GetValueFromY(this->GetYFromValue(this->GetEventValue(midi_event)) + y_offset));
}


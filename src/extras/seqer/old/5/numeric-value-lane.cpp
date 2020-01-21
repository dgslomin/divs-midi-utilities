
#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

NumericValueLane::NumericValueLane(Window* window): Lane(window)
{
}

NumericValueLane::~NumericValueLane()
{
}

void NumericValueLane::PaintBackground(wxDC& dc, int width, int height)
{
	dc.SetBackground(this->window->application->background_brush);
	dc.Clear();
}

void NumericValueLane::PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int selected_events_y_offset)
{
	wxRect bounds(0, 0, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->ShouldIncludeEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event, selected_events_x_offset, selected_events_y_offset);

			if (rect.Intersects(bounds))
			{
				if (MidiFileEvent_isSelected(midi_event))
				{
					dc.SetPen(this->window->application->selected_event_pen);
					dc.SetBrush(this->window->application->selected_event_brush);
				}
				else
				{
					dc.SetPen(this->window->application->unselected_event_pen);
					dc.SetBrush(this->window->application->unselected_event_brush);
				}

				dc.DrawRectangle(rect);
			}
		}
	}
}

MidiFileEvent_t NumericValueLane::GetEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (this->ShouldIncludeEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event, 0, 0);
			if (rect.Contains(x, y)) return midi_event;
		}
	}

	return NULL;
}

MidiFileEvent_t NumericValueLane::AddEventAtXY(int x, int y)
{
	return this->AddEvent(this->window->GetTickFromX(x), this->GetValueFromY(y));
}

void NumericValueLane::MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset)
{
	MidiFileEvent_setTick(midi_event, this->window->GetTickFromX(this->window->GetXFromTick(MidiFileEvent_getTick(midi_event)) + x_offset));
	this->SetEventValue(midi_event, this->GetValueFromY(this->GetYFromValue(this->GetEventValue(midi_event)) + y_offset));
}

void NumericValueLane::SelectEventsInRect(int x, int y, int width, int height)
{
	wxRect bounds(x, y, width, height);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->ShouldIncludeEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event, 0, 0);

			if (rect.Intersects(bounds))
			{
				MidiFileEvent_setSelected(midi_event, 1);
			}
		}
	}
}

wxRect NumericValueLane::GetRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset)
{
	int x = this->window->GetXFromTick(MidiFileEvent_getTick(midi_event)) + selected_events_x_offset;
	int y = this->GetYFromValue(this->GetEventValue(midi_event)) + selected_events_y_offset;
	return wxRect(x - 2, y - 2, 5, 5);
}

int NumericValueLane::GetYFromValue(float value)
{
	return (int)(value * this->pixels_per_value) - this->scroll_y;
}

float NumericValueLane::GetValueFromY(int y)
{
	return (float)(y + this->scroll_y) / this->pixels_per_value;
}


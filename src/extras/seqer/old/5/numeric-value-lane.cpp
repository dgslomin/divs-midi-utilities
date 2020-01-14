
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

void NumericValueLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	wxRect bounds = wxRect(this->GetSize());

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (this->ShouldIncludeEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event);
			if (rect.Intersects(bounds)) dc.DrawRectangle(rect);
		}
	}
}

MidiFileEvent_t NumericValueLane::GetEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if (this->ShouldIncludeEvent(midi_event))
		{
			wxRect rect = this->GetRectFromEvent(midi_event);
			if (rect.Contains(x, y)) return midi_event;
		}
	}

	return NULL;
}

wxRect NumericValueLane::GetRectFromEvent(MidiFileEvent_t midi_event)
{
	int x = this->window->GetXFromTick(MidiFileEvent_getTick(midi_event));
	int y = this->GetYFromValue(this->GetValueFromEvent(midi_event));
	return wxRect(x - 1, y - 1, 3, 3);
}

int NumericValueLane::GetYFromValue(int value)
{
	return (value * this->pixels_per_value) - this->scroll_y;
}

int NumericValueLane::GetValueFromY(int y)
{
	return (y + this->scroll_y) / this->pixels_per_value;
}


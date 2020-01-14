
#include <wx/wx.h>
#include "control-change-lane.h"
#include "lane.h"
#include "midifile.h"
#include "window.h"

ControlChangeLane::ControlChangeLane(Window* window): Lane(window)
{
}

ControlChangeLane::~ControlChangeLane()
{
}

void ControlChangeLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	wxRect bounds = wxRect(this->GetSize());

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if ((MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) && (MidiFileControlChangeEvent_getNumber(midi_event) == this->controller_number))
		{
			wxRect rect = this->GetRectFromEvent(midi_event);
			if (rect.Intersects(bounds)) dc.DrawRectangle(rect);
		}
	}
}

MidiFileEvent_t ControlChangeLane::GetEventFromXY(int x, int y)
{
	for (MidiFileEvent_t midi_event = MidiFile_getLastEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getPreviousEventInFile(midi_event))
	{
		if ((MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) && (MidiFileControlChangeEvent_getNumber(midi_event) == this->controller_number))
		{
			wxRect rect = this->GetRectFromEvent(midi_event);
			if (rect.Contains(x, y)) return midi_event;
		}
	}

	return NULL;
}

wxRect ControlChangeLane::GetRectFromEvent(MidiFileEvent_t midi_event)
{
	int x = this->window->GetXFromTick(MidiFileEvent_getTick(midi_event));
	int y = this->GetYFromValue(MidiFileControlChangeEvent_getValue(midi_event));
	return wxRect(x - 1, y - 1, 3, 3);
}

int ControlChangeLane::GetYFromValue(int value)
{
	return (value * this->pixels_per_value) - this->scroll_y;
}

int ControlChangeLane::GetValueFromY(int y)
{
	return (y + this->scroll_y) / this->pixels_per_value;
}


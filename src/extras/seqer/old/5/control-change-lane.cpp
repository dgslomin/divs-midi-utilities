
#include <wx/wx.h>

ControlChangeLane::ControlChangeLane(Window* window): wxWindow(window, wxID_ANY)
{
	this->Bind(wxEVT_PAINT, &OnPaint);
}

ControlChangeLane::~ControlChangeLane()
{
}

void ControlChangeLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEventInFile(this->window->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isControlChangeEvent(midi_event) && (MidiFileControlChangeEvent_getNumber(midi_event) == this->controller_number))
		{
			int x = this->window->GetXForTick(MidiFileEvent_getTick(midi_event));
			if ((x < 0) || (x > this->window->width)) continue;
			int y = this->GetYForValue(MidiFileControlChangeEvent_getValue(midi_event)); 
			if ((y < 0) || (y > this->height)) continue;
			dc.drawRectangle(x, y, this->value_height, this->value_height);
		}
	}
}

int ControlChangeLane::GetYForValue(int note)
{
	return (note * this->value_height) - scroll_y;
}

int ControlChangeLane::GetValueForY(int y)
{
	return (y + scroll_y) / this->value_height;
}


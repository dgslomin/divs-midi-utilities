
#include <wx/wx.h>
#include "label.h"
#include "midifile.h"

Label::Label(MidiEvent_t midi_event, wxString text)
{
	this->midi_event = midi_event;
	this->label = text;
}

Label::~Label()
{
}



#include <wx/wx.h>
#include "label.h"
#include "midifile.h"

Label::Label(MidiFileEvent_t midi_event, wxString text)
{
	this->midi_event = midi_event;
	this->text = text;
}

Label::~Label()
{
}


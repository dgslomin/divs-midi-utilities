#ifndef LABEL_INCLUDED
#define LABEL_INCLUDED

class Label;

#include <wx/wx.h>
#include "midifile.h"

class Label
{
public:
	MidiFileEvent_t midi_event;
	wxString text;
	int x;
	int width;
	int row;
	
	Label(MidiFileEvent_t midi_event, wxString text)
	~Label();
};

#endif

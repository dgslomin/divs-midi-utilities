#ifndef SEQUENCE_INCLUDED
#define SEQUENCE_INCLUDED

class Sequence;

#include <wx/wx.h>
#include "midifile.h"
#include "window.h"

class Sequence
{
	std::list<Window*> windows;
	wxCommandProcessor* undo_command_processor;
	wxString filename;
	MidiFile_t midi_file;
	bool is_modified;

	Sequence();
	~Sequence();
	void AddWindow(Window* window);
	void RemoveWindow(window* window);
	void RefreshData();
	void RefreshDisplay();
	bool Save();
	bool SaveAs(wxString filename);
};

#endif

#ifndef SEQUENCE_INCLUDED
#define SEQUENCE_INCLUDED

class Sequence;

#include <list>
#include <wx/wx.h>
#include <wx/cmdproc.h>
#include "midifile.h"
#include "window.h"

class Sequence
{
public:
	std::list<Window*> windows;
	wxCommandProcessor* undo_command_processor;
	wxString filename;
	MidiFile_t midi_file;
	bool is_modified;

	Sequence();
	~Sequence();
	virtual void AddWindow(Window* window);
	virtual void RemoveWindow(Window* window);
	virtual void RefreshData();
	virtual void RefreshDisplay();
	virtual bool Save();
	virtual bool SaveAs(wxString filename);
};

#endif


#include <wx/wx.h>
#include <midifile.h>
#include "window.h"

Sequence::Sequence()
{
	this->undo_command_processor = new wxCommandProcessor();
	this->filename = wxEmptyString;
	this->midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	this->is_modified = false;
}

Sequence::~Sequence()
{
	MidiFile_free(this->midi_file);
	delete this->undo_command_processor;
}

void Sequence::AddWindow(Window* window)
{
	this->windows.push_back(window);
}

void Sequence::RemoveWindow(Window* window)
{
	this->windows.remove(window);
	if (this->windows.empty()) delete this;
}

void Sequence::RefreshData()
{
	this->is_modified = true;
	for (std::list<Window*>::iterator window_iterator = this->windows.begin(); window_iterator != this->windows.end(); window_iterator++) (*window_iterator)->RefreshData();
}

void Sequence::RefreshDisplay()
{
	for (std::list<Window*>::iterator window_iterator = this->windows.begin(); window_iterator != this->windows.end(); window_iterator++) (*window_iterator)->RefreshDisplay();
}

bool Sequence::Save()
{
	if (this->filename == wxEmptyString) return false;

	if (MidiFile_save(this->midi_file, (char *)(this->filename.ToStdString().c_str())) == 0)
	{
		this->is_modified = false;
		this->RefreshDisplay();
		return true;
	}
	else
	{
		return false;
	}
}

bool Sequence::SaveAs(wxString filename)
{
	if (MidiFile_save(this->midi_file, (char *)(filename.ToStdString().c_str())) == 0)
	{
		this->filename = filename;
		this->is_modified = false;
		this->RefreshDisplay();
		return true;
	}
	else
	{
		return false;
	}
}


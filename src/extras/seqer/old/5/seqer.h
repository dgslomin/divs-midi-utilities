#ifndef SEQER_INCLUDED
#define SEQER_INCLUDED

class Application;
class Window;
class TrackListWindow;
class PianoRollWindow;
class EventListWindow;
class Sequence;

#include <wx/wx.h>
#include <wx/fontpicker.h>

class Application: public wxApp
{
public:
	bool OnInit();
};

class Window: public wxFrame
{
public:
	Application* application;
	Sequence* sequence;

	Window(Application* application, Window* existing_window = NULL);
	~Window();
	bool SaveChangesIfNeeded();
	bool SaveChanges();
	bool Save();
	bool SaveAs();
};

class TrackListWindow: public Window
{
};

class PianoRollWindow: public Window
{
};

class EventListWindow: public Window
{
};

class Sequence: 
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
};

#endif

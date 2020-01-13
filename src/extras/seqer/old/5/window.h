#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

class Window;

#include <wx/wx.h>
#include "application.h"
#include "sequence.h"

class Window: public wxFrame
{
public:
	Application* application;
	Sequence* sequence;

	Window(Application* application, Window* existing_window = NULL);
	~Window();
	bool IsModified();
	bool IsLastWindowForSequence();
	wxString GetFilename();
	void New();
	void NewWindow();
	void Open();
	bool Load(wxString filename);
	bool SaveChangesIfNeeded();
	bool SaveChanges();
	bool Save();
	bool SaveAs();
	bool Quit(wxCommandEvent& event);
};

#endif

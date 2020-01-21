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
	bool use_linear_time;
	int pixels_per_beat;
	int pixels_per_second;
	int scroll_x;

	Window(Application* application, Window* existing_window = NULL);
	~Window();
	virtual void CreateMenuBar();
	virtual void CreateFileMenu(wxMenuBar* menu_bar);
	virtual void CreateEditMenu(wxMenuBar* menu_bar);
	virtual void CreateTransportMenu(wxMenuBar* menu_bar);
	virtual void CreateToolsMenu(wxMenuBar* menu_bar);
	virtual void CreateHelpMenu(wxMenuBar* menu_bar);
	virtual bool IsModified();
	virtual bool IsLastWindowForSequence();
	virtual wxString GetFilename();
	virtual void New();
	virtual void NewWindow();
	virtual void Open();
	virtual bool Load(wxString filename);
	virtual bool SaveChangesIfNeeded();
	virtual bool SaveChanges();
	virtual bool Save();
	virtual bool SaveAs();
	virtual void Quit();
	virtual void SelectAll();
	virtual void SelectNone();
	virtual void FocusPropertyEditor();
	virtual void RefreshData();
	virtual void RefreshDisplay();
	virtual int GetXFromTick(long tick);
	virtual long GetTickFromX(int x);
};

#endif

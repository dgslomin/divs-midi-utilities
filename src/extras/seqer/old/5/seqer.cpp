
#include <wx/wx.h>
#include <midifile.h>
#include "seqer.h"

#ifndef __WXMSW__
#include "seqer.xpm"
#endif

enum
{
	SEQER_ID_NEW_WINDOW = wxID_HIGHEST + 1,
	SEQER_ID_HIGHEST
};

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	if (wxGetEnv("SEQER_DEBUG", NULL)) wxMessageBox("Attach debugger now.", "Seqer", wxOK);

	Window* window = new Window(this);
	window->Show(true);

	return true;
}

Window::Window(Application* application, Window* existing_window): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->application = application;
	this->SetIcon(wxICON(seqer));
	this->CreateStatusBar();

	wxMenuBar* menu_bar = new wxMenuBar();
		wxMenu* file_menu = new wxMenu();
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W");
		menu_bar->Append(file_menu, "&File");
		wxMenu* edit_menu = new wxMenu();
			edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
			edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Shift+Z");
			edit_menu->AppendSeparator();
			edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
			edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
			edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
			edit_menu->Append(wxID_DELETE, "&Delete\tDel");
			edit_menu->AppendSeparator();
			edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
			edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");
		menu_bar->Append(edit_menu, "&Edit");
		wxMenu* view_menu = new wxMenu();
			view_menu->Append(SEQER_ID_VIEW_TRACK_LIST, "&Track List", "", wxITEM_RADIO);
			view_menu->Append(SEQER_ID_VIEW_PIANO_ROLL, "&Piano Roll", "", wxITEM_RADIO);
			view_menu->Append(SEQER_ID_VIEW_EVENT_LIST, "&Event List", "", wxITEM_RADIO);
		menu_bar->Append(view_menu, "&View");
		wxMenu* track_menu = new wxMenu();
			track_menu->Append(SEQER_ID_NEW_TRACK, "&New Track");
			track_menu->AppendSeparator();
			track_menu->Append(SEQER_ID_TRACK_RENAME, "&Rename");
			track_menu->Append(SEQER_ID_TRACK_INPUT_PORT, "&Input Port");
			track_menu->Append(SEQER_ID_TRACK_OUTPUT_PORT, "&Output Port");
			track_menu->AppendSeparator();
			track_menu->Append(SEQER_ID_ARM_TRACK, "&Arm", "", wxITEM_CHECK);
			track_menu->Append(SEQER_ID_MUTE_TRACK, "&Mute", "", wxITEM_CHECK);
			track_menu->Append(SEQER_ID_SOLO_TRACK, "&Solo", "", wxITEM_CHECK);
		menu_bar->Append(track_menu, "&Track");
		wxMenu* transport_menu = new wxMenu();
			transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
			transport_menu->Append(SEQER_ID_STOP, "&Stop\tShift+Space");
			transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
			transport_menu->AppendSeparator();
#ifdef __WXOSX__
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tAlt+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tAlt+Left");
#else
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+Right");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+Left");
#endif
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go to &Marker...\tCtrl+Shift+M");
		menu_bar->Append(transport_menu, "Trans&port");
		wxMenu* tools_menu = new wxMenu();
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+R");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
#ifdef __WXOSX__
			tools_menu->Append(wxID_PREFERENCES, "&Preferences...\tCtrl+,");
#else
			tools_menu->Append(wxID_PREFERENCES, "&Preferences...");
#endif
		menu_bar->Append(tools_menu, "&Tools");
		wxMenu* help_menu = new wxMenu();
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About Seqer");
		menu_bar->Append(help_menu, "&Help");
	this->SetMenuBar(menu_bar);

	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
	this->Bind(wxEVT_MENU_HIGHLIGHT, [=](wxMenuEvent&) {});

	this->sequence_editor = new SequenceEditor(this, (existing_window == NULL) ? NULL : existing_window->sequence_editor);
}

Window::~Window()
{
	delete this->sequence_editor;
}


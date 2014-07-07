
#include <wx/wx.h>
#include <midifile.h>

enum
{
	SEQER_ID_SELECT_CURRENT = wxID_HIGHEST + 1,
	SEQER_ID_SELECT_NONE,
	SEQER_ID_SMALL_INCREASE,
	SEQER_ID_SMALL_DECREASE,
	SEQER_ID_LARGE_INCREASE,
	SEQER_ID_LARGE_DECREASE,
	SEQER_ID_ZOOM,
	SEQER_ID_FILTER,
	SEQER_ID_INSERT_NOTE_A,
	SEQER_ID_INSERT_NOTE_B,
	SEQER_ID_INSERT_NOTE_C,
	SEQER_ID_INSERT_NOTE_D,
	SEQER_ID_INSERT_NOTE_E,
	SEQER_ID_INSERT_NOTE_F,
	SEQER_ID_INSERT_NOTE_G,
	SEQER_ID_INSERT_CONTROL_CHANGE,
	SEQER_ID_INSERT_PROGRAM_CHANGE,
	SEQER_ID_INSERT_AFTERTOUCH,
	SEQER_ID_INSERT_PITCH_BEND,
	SEQER_ID_INSERT_SYSTEM_EXCLUSIVE,
	SEQER_ID_INSERT_LYRIC,
	SEQER_ID_INSERT_MARKER,
	SEQER_ID_INSERT_TEMPO,
	SEQER_ID_INSERT_TIME_SIGNATURE,
	SEQER_ID_INSERT_KEY_SIGNATURE,
	SEQER_ID_PLAY,
	SEQER_ID_RECORD,
	SEQER_ID_STOP,
	SEQER_ID_STEP_RECORD,
	SEQER_ID_NEXT_MARKER,
	SEQER_ID_PREVIOUS_MARKER,
	SEQER_ID_GO_TO_MARKER,
	SEQER_ID_PORTS,
	SEQER_ID_RECORD_MACRO,
	SEQER_ID_MACROS,
	SEQER_ID_EXTERNAL_UTILITY,
	SEQER_ID_HIGHEST
};

class Seqer: public wxApp
{
public:
	MidiFile_t midi_file;
	wxFrame* window;
	class Canvas* canvas;

	bool OnInit();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
};

class Canvas: public wxScrolledCanvas
{
public:
	class Seqer* seqer;

	Canvas(Seqer* seqer);
	void OnDraw(wxDC& dc);
};

bool Seqer::OnInit()
{
	this->midi_file = NULL;

	this->window = new wxFrame((wxFrame*)(NULL), -1, "Seqer", wxDefaultPosition, wxSize(640, 480));
	this->SetTopWindow(this->window);

	wxMenuBar* menu_bar = new wxMenuBar();
	this->window->SetMenuBar(menu_bar);
	this->window->Connect(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(Seqer::OnMenuHighlight));

	wxMenu* file_menu = new wxMenu();
	menu_bar->Append(file_menu, "&File");
	file_menu->Append(wxID_NEW, "&New\tCtrl+N");
	file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
	this->Connect(wxID_OPEN, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnFileOpen));
	file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
	file_menu->Append(wxID_SAVEAS, "Save &As...");
	file_menu->Append(wxID_REVERT, "&Revert");
#ifndef __WXOSX__
	file_menu->AppendSeparator();
	file_menu->Append(wxID_EXIT, "E&xit");
#endif
	this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnExit));

	wxMenu* edit_menu = new wxMenu();
	menu_bar->Append(edit_menu, "&Edit");
	edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
	edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Shift+Z");
	edit_menu->AppendSeparator();
	edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
	edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
	edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
	edit_menu->Append(wxID_DELETE, "&Delete\tDel");
	edit_menu->AppendSeparator();
	edit_menu->Append(SEQER_ID_SELECT_CURRENT, "Select Current\tShift+Right");
	edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
	edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");
	edit_menu->AppendSeparator();
	edit_menu->Append(SEQER_ID_SMALL_INCREASE, "Small &Increase\t]");
	edit_menu->Append(SEQER_ID_SMALL_DECREASE, "Small &Decrease\t[");
	edit_menu->Append(SEQER_ID_LARGE_INCREASE, "Large &Increase\tShift+]");
	edit_menu->Append(SEQER_ID_LARGE_DECREASE, "Large &Decrease\tShift+[");

	wxMenu* view_menu = new wxMenu();
	menu_bar->Append(view_menu, "&View");
	view_menu->Append(SEQER_ID_ZOOM, "&Zoom...");
	view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++");
	view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-");
	view_menu->AppendSeparator();
	view_menu->Append(SEQER_ID_FILTER, "&Filter...");

	wxMenu* insert_menu = new wxMenu();
	menu_bar->Append(insert_menu, "&Insert");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_A, "&Note A\tA");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_B, "&Note B\tB");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_C, "&Note C\tC");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_D, "&Note D\tD");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_E, "&Note E\tE");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_F, "&Note F\tF");
	insert_menu->Append(SEQER_ID_INSERT_NOTE_G, "&Note G\tG");
	insert_menu->AppendSeparator();
	insert_menu->Append(SEQER_ID_INSERT_CONTROL_CHANGE, "&Control Change\tShift+C");
	insert_menu->Append(SEQER_ID_INSERT_PROGRAM_CHANGE, "&Program Change\tShift+P");
	insert_menu->Append(SEQER_ID_INSERT_AFTERTOUCH, "&Aftertouch\tShift+A");
	insert_menu->Append(SEQER_ID_INSERT_PITCH_BEND, "Pitch &Bend\tShift+B");
	insert_menu->Append(SEQER_ID_INSERT_SYSTEM_EXCLUSIVE, "&System Exclusive\tShift+S");
	insert_menu->AppendSeparator();
	insert_menu->Append(SEQER_ID_INSERT_LYRIC, "&Lyric\tShift+L");
	insert_menu->Append(SEQER_ID_INSERT_MARKER, "&Marker\tShift+M");
	insert_menu->Append(SEQER_ID_INSERT_TEMPO, "&Tempo\tShift+T");
	insert_menu->Append(SEQER_ID_INSERT_TIME_SIGNATURE, "T&ime Signature\tShift+I");
	insert_menu->Append(SEQER_ID_INSERT_KEY_SIGNATURE, "&Key Signature\tShift+K");

	wxMenu* transport_menu = new wxMenu();
	menu_bar->Append(transport_menu, "Trans&port");
	transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
	transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
	transport_menu->Append(SEQER_ID_STOP, "&Stop\tEsc");
	transport_menu->Append(SEQER_ID_STEP_RECORD, "S&tep Record\tCtrl+T", "", wxITEM_CHECK);
	transport_menu->AppendSeparator();
	transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+]");
	transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+[");
	transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go To &Marker...\tCtrl+M");
	transport_menu->AppendSeparator();
	transport_menu->Append(SEQER_ID_PORTS, "P&orts...");

	wxMenu* tools_menu = new wxMenu();
	menu_bar->Append(tools_menu, "&Tools");
	tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+M");
	tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
	tools_menu->AppendSeparator();
	tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");

	wxMenu* help_menu = new wxMenu();
	menu_bar->Append(help_menu, "&Help");
	help_menu->Append(wxID_HELP_CONTENTS, "&Manual");
	help_menu->Append(wxID_ABOUT, "&About");

	this->canvas = new Canvas(this);

	this->window->CreateStatusBar();

	this->window->Show(true);
	return true;
}

void Seqer::OnMenuHighlight(wxMenuEvent& WXUNUSED(event))
{
	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
}

void Seqer::OnFileOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog* file_dialog = new wxFileDialog(this->window, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (file_dialog->ShowModal() == wxID_OK)
	{
		if (this->midi_file != NULL) MidiFile_free(this->midi_file);
		this->midi_file = MidiFile_load((char*)(file_dialog->GetPath().ToStdString().c_str()));
	}

	delete file_dialog;
}

void Seqer::OnExit(wxCommandEvent& WXUNUSED(event))
{
	this->window->Close(true);
}

Canvas::Canvas(Seqer* seqer): wxScrolledCanvas(seqer->window)
{
	this->seqer = seqer;
	wxClientDC dc(this);
	this->SetScrollbars(dc.GetCharWidth(), dc.GetCharHeight(), 0, 100);
}

void Canvas::OnDraw(wxDC& dc)
{
	dc.DrawText("The quick brown fox jumps over the lazy dog.", 0, 0);
}

IMPLEMENT_APP(Seqer)


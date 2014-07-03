
#include <wx/wx.h>
#include <midifile.h>

enum
{
	SEQER_ID_SELECT_NONE = wxID_HIGHEST + 1,
	SEQER_ID_ZOOM,
	SEQER_ID_FILTER,
	SEQER_ID_INSERT_NOTE,
	SEQER_ID_INSERT_CONTROL_CHANGE,
	SEQER_ID_INSERT_PROGRAM_CHANGE,
	SEQER_ID_INSERT_AFTERTOUCH,
	SEQER_ID_INSERT_CHANNEL_PRESSURE,
	SEQER_ID_INSERT_PITCH_WHEEL,
	SEQER_ID_INSERT_SYSTEM_EXCLUSIVE,
	SEQER_ID_INSERT_LYRIC,
	SEQER_ID_INSERT_MARKER,
	SEQER_ID_INSERT_TEMPO,
	SEQER_ID_INSERT_TIME_SIGNATURE,
	SEQER_ID_INSERT_KEY_SIGNATURE,
	SEQER_ID_PLAY,
	SEQER_ID_RECORD,
	SEQER_ID_STOP,
	SEQER_ID_HIGHEST
};

class SequenceView: public wxScrolledCanvas
{
public:
    SequenceView(wxWindow* parent): wxScrolledCanvas(parent)
    {
        wxClientDC dc(this);
        this->SetScrollbars(dc.GetCharWidth(), dc.GetCharHeight(), 0, 100);
    }

    void OnDraw(wxDC& dc)
    {
        dc.DrawText("The quick brown fox jumps over the lazy dog.", 0, 0);
    }
};

class Seqer: public wxApp
{
public:
	MidiFile_t midi_file;
	wxFrame* main_window;
    SequenceView* sequence_view;

    bool OnInit()
    {
	    this->midi_file = NULL;

	    this->main_window = new wxFrame((wxFrame*)(NULL), -1, "Seqer", wxDefaultPosition, wxSize(640, 480));
	    this->SetTopWindow(this->main_window);

	    wxMenuBar* menu_bar = new wxMenuBar();
	    this->main_window->SetMenuBar(menu_bar);
	    this->main_window->Connect(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(Seqer::OnMenuHighlight));

	    wxMenu* file_menu = new wxMenu();
	    menu_bar->Append(file_menu, "&File");
	    file_menu->Append(wxID_NEW, "&New\tCtrl+N");
	    file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O");
	    this->Connect(wxID_OPEN, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnFileOpen));
	    file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
	    file_menu->Append(wxID_SAVEAS, "Save &As...");
	    file_menu->Append(wxID_REVERT, "&Revert");
	    file_menu->AppendSeparator();
	    file_menu->Append(wxID_EXIT, "E&xit");
	    this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Seqer::OnExit));

	    wxMenu* edit_menu = new wxMenu();
	    menu_bar->Append(edit_menu, "&Edit");
	    edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
#ifdef __WXMSW__
	    edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Y");
#else
	    edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Shift+Z");
#endif
	    edit_menu->AppendSeparator();
	    edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
	    edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
	    edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
	    edit_menu->AppendSeparator();
	    edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
	    edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");

	    wxMenu* view_menu = new wxMenu();
	    menu_bar->Append(view_menu, "&View");
	    view_menu->Append(SEQER_ID_ZOOM, "&Zoom...");
	    view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++");
	    view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-");
	    view_menu->AppendSeparator();
	    view_menu->Append(SEQER_ID_FILTER, "&Filter...");

	    wxMenu* insert_menu = new wxMenu();
	    menu_bar->Append(insert_menu, "&Insert");
	    insert_menu->Append(SEQER_ID_INSERT_NOTE, "&Note");
	    insert_menu->Append(SEQER_ID_INSERT_CONTROL_CHANGE, "&Control Change");
	    insert_menu->Append(SEQER_ID_INSERT_PROGRAM_CHANGE, "&Program Change");
	    insert_menu->Append(SEQER_ID_INSERT_AFTERTOUCH, "&Aftertouch");
	    insert_menu->Append(SEQER_ID_INSERT_CHANNEL_PRESSURE, "Channel P&ressure");
	    insert_menu->Append(SEQER_ID_INSERT_PITCH_WHEEL, "Pitch &Wheel");
	    insert_menu->Append(SEQER_ID_INSERT_SYSTEM_EXCLUSIVE, "&System Exclusive");
	    insert_menu->AppendSeparator();
	    insert_menu->Append(SEQER_ID_INSERT_LYRIC, "&Lyric");
	    insert_menu->Append(SEQER_ID_INSERT_MARKER, "&Marker");
	    insert_menu->Append(SEQER_ID_INSERT_TEMPO, "&Tempo");
	    insert_menu->Append(SEQER_ID_INSERT_TIME_SIGNATURE, "T&ime Signature");
	    insert_menu->Append(SEQER_ID_INSERT_KEY_SIGNATURE, "&Key Signature");

	    wxMenu* transport_menu = new wxMenu();
	    menu_bar->Append(transport_menu, "Trans&port");
	    transport_menu->Append(SEQER_ID_PLAY, "&Play");
	    transport_menu->Append(SEQER_ID_RECORD, "&Record");
	    transport_menu->Append(SEQER_ID_STOP, "&Stop");

	    wxMenu* tools_menu = new wxMenu();
	    menu_bar->Append(tools_menu, "&Tools");

	    wxMenu* help_menu = new wxMenu();
	    menu_bar->Append(help_menu, "&Help");
	    help_menu->Append(wxID_HELP_CONTENTS, "&Help Topics");
	    help_menu->Append(wxID_ABOUT, "&About Seqer");

        this->sequence_view = new SequenceView(this->main_window);

	    this->main_window->CreateStatusBar();

	    this->main_window->Show(true);
	    return true;
    }

    void OnMenuHighlight(wxMenuEvent& WXUNUSED(event))
    {
	    // This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
    }

    void OnFileOpen(wxCommandEvent& WXUNUSED(event))
    {
	    wxFileDialog* file_dialog = new wxFileDialog(this->main_window, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	    if (file_dialog->ShowModal() == wxID_OK)
	    {
		    if (this->midi_file != NULL) MidiFile_free(this->midi_file);
		    this->midi_file = MidiFile_load((char *)(file_dialog->GetPath().ToStdString().c_str()));
	    }

	    delete file_dialog;
    }

    void OnExit(wxCommandEvent& WXUNUSED(event))
    {
	    this->main_window->Close(true);
    }
};

IMPLEMENT_APP(Seqer)


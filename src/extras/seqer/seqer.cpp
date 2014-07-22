
#include <vector>
#include <wx/wx.h>
#include <midifile.h>

class Application: public wxApp
{
public:
	bool OnInit();
};

class Window: public wxFrame
{
public:
	MidiFile_t midi_file;
	std::vector<class Row> rows;
	class Canvas* canvas;
	long row_height;
	long char_width;

	Window();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void PrepareRows();
};

class Row
{
public:
	int step;
	MidiFileEvent_t event;

	Row(int step, MidiFileEvent_t event);
};

class Canvas: public wxScrolledCanvas
{
public:
	class Window* window;

	Canvas(Window* window);
	void OnDraw(wxDC& dc);
};

enum
{
	SEQER_ID_NEW_WINDOW = wxID_HIGHEST + 1,
	SEQER_ID_SELECT_CURRENT,
	SEQER_ID_SELECT_NONE,
	SEQER_ID_ENTER_VALUE,
	SEQER_ID_SMALL_INCREASE,
	SEQER_ID_SMALL_DECREASE,
	SEQER_ID_LARGE_INCREASE,
	SEQER_ID_LARGE_DECREASE,
	SEQER_ID_QUANTIZE,
	SEQER_ID_ZOOM,
	SEQER_ID_FILTER,
	SEQER_ID_VIEW_COLUMN_1,
	SEQER_ID_VIEW_COLUMN_2,
	SEQER_ID_VIEW_COLUMN_3,
	SEQER_ID_VIEW_COLUMN_4,
	SEQER_ID_VIEW_COLUMN_5,
	SEQER_ID_VIEW_COLUMN_6,
	SEQER_ID_VIEW_COLUMN_7,
	SEQER_ID_VIEW_COLUMN_8,
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
	SEQER_ID_INSERT_TEXT,
	SEQER_ID_INSERT_LYRIC,
	SEQER_ID_INSERT_MARKER,
	SEQER_ID_INSERT_PORT,
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

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
	Window* window = new Window();
	this->SetTopWindow(window);
	window->Show(true);
	return true;
}

Window::Window(): wxFrame((wxFrame*)(NULL), -1, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->midi_file = NULL;

	wxMenuBar* menu_bar = new wxMenuBar(); this->SetMenuBar(menu_bar); this->Connect(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(Window::OnMenuHighlight));
		wxMenu* file_menu = new wxMenu(); menu_bar->Append(file_menu, "&File");
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O"); this->Connect(wxID_OPEN, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnFileOpen));
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...");
			file_menu->Append(wxID_REVERT, "&Revert");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W");
#if defined(__WXMSW__)
			file_menu->Append(wxID_EXIT, "E&xit\tAlt+F4"); this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnExit));
#elif defined(__WXGTK__)
			file_menu->Append(wxID_EXIT, "&Quit\tCtrl+Q"); this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnExit));
#elif defined(__WXOSX__)
			// MacOS automatically creates a quit item in the application menu.
			this->Connect(wxID_EXIT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnExit));
#endif
		wxMenu* edit_menu = new wxMenu(); menu_bar->Append(edit_menu, "&Edit");
			edit_menu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
			edit_menu->Append(wxID_REDO, "&Redo\tCtrl+Shift+Z");
			edit_menu->AppendSeparator();
			edit_menu->Append(wxID_CUT, "Cu&t\tCtrl+X");
			edit_menu->Append(wxID_COPY, "&Copy\tCtrl+C");
			edit_menu->Append(wxID_PASTE, "&Paste\tCtrl+V");
			edit_menu->Append(wxID_DELETE, "&Delete\tDel");
			edit_menu->AppendSeparator();
			edit_menu->Append(SEQER_ID_SELECT_CURRENT, "Se&lect Current\tShift+Right");
			edit_menu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
			edit_menu->Append(SEQER_ID_SELECT_NONE, "Select &None\tCtrl+Shift+A");
			edit_menu->AppendSeparator();
			edit_menu->Append(SEQER_ID_ENTER_VALUE, "Enter &Value\tEnter");
			edit_menu->Append(SEQER_ID_SMALL_INCREASE, "Small &Increase\t]");
			edit_menu->Append(SEQER_ID_SMALL_DECREASE, "Small D&ecrease\t[");
			edit_menu->Append(SEQER_ID_LARGE_INCREASE, "Large &Increase\tShift+]");
			edit_menu->Append(SEQER_ID_LARGE_DECREASE, "Large D&ecrease\tShift+[");
			edit_menu->Append(SEQER_ID_QUANTIZE, "&Quantize\t=");
		wxMenu* view_menu = new wxMenu(); menu_bar->Append(view_menu, "&View");
			view_menu->Append(SEQER_ID_ZOOM, "&Zoom...\tCtrl+Shift+M");
			view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++");
			view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-");
			view_menu->AppendSeparator();
			view_menu->Append(SEQER_ID_FILTER, "&Filter...\tCtrl+Shift+F");
			view_menu->AppendSeparator();
			wxMenu* view_column_menu = new wxMenu(); view_menu->AppendSubMenu(view_column_menu, "&Columns");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_1, "Column &1\tCtrl+1");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_2, "Column &2\tCtrl+2");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_3, "Column &3\tCtrl+3");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_4, "Column &4\tCtrl+4");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_5, "Column &5\tCtrl+5");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_6, "Column &6\tCtrl+6");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_7, "Column &7\tCtrl+7");
				view_column_menu->Append(SEQER_ID_VIEW_COLUMN_8, "Column &8\tCtrl+8");
		wxMenu* insert_menu = new wxMenu(); menu_bar->Append(insert_menu, "&Insert");
			wxMenu* insert_note_menu = new wxMenu(); insert_menu->AppendSubMenu(insert_note_menu, "&Notes");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_A, "Note &A\tA");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_B, "Note &B\tB");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_C, "Note &C\tC");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_D, "Note &D\tD");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_E, "Note &E\tE");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_F, "Note &F\tF");
				insert_note_menu->Append(SEQER_ID_INSERT_NOTE_G, "Note &G\tG");
			insert_menu->AppendSeparator();
			insert_menu->Append(SEQER_ID_INSERT_CONTROL_CHANGE, "&Control Change\tShift+C");
			insert_menu->Append(SEQER_ID_INSERT_PROGRAM_CHANGE, "&Program Change\tShift+P");
			insert_menu->Append(SEQER_ID_INSERT_AFTERTOUCH, "&Aftertouch\tShift+A");
			insert_menu->Append(SEQER_ID_INSERT_PITCH_BEND, "Pitch &Bend\tShift+B");
			insert_menu->Append(SEQER_ID_INSERT_SYSTEM_EXCLUSIVE, "&System Exclusive\tShift+S");
			insert_menu->AppendSeparator();
			insert_menu->Append(SEQER_ID_INSERT_TEXT, "Te&xt\tShift+X");
			insert_menu->Append(SEQER_ID_INSERT_LYRIC, "&Lyric\tShift+L");
			insert_menu->Append(SEQER_ID_INSERT_MARKER, "&Marker\tShift+M");
			insert_menu->Append(SEQER_ID_INSERT_PORT, "P&ort\tShift+O");
			insert_menu->Append(SEQER_ID_INSERT_TEMPO, "&Tempo\tShift+T");
			insert_menu->Append(SEQER_ID_INSERT_TIME_SIGNATURE, "T&ime Signature\tShift+I");
			insert_menu->Append(SEQER_ID_INSERT_KEY_SIGNATURE, "&Key Signature\tShift+K");
		wxMenu* transport_menu = new wxMenu(); menu_bar->Append(transport_menu, "Trans&port");
			transport_menu->Append(SEQER_ID_PLAY, "&Play\tSpace");
			transport_menu->Append(SEQER_ID_RECORD, "&Record\tCtrl+R");
			transport_menu->Append(SEQER_ID_STOP, "&Stop\tShift+Space");
			transport_menu->Append(SEQER_ID_STEP_RECORD, "S&tep Record\tCtrl+T", "", wxITEM_CHECK);
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_NEXT_MARKER, "&Next Marker\tCtrl+]");
			transport_menu->Append(SEQER_ID_PREVIOUS_MARKER, "Pre&vious Marker\tCtrl+[");
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go To &Marker...\tCtrl+M");
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_PORTS, "P&orts...\tCtrl+Shift+P");
		wxMenu* tools_menu = new wxMenu(); menu_bar->Append(tools_menu, "&Tools");
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+Q");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
		wxMenu* help_menu = new wxMenu(); menu_bar->Append(help_menu, "&Help");
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About"); this->Connect(wxID_ABOUT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnAbout));

	this->canvas = new Canvas(this);

	this->CreateStatusBar();
	this->PrepareRows();
}

void Window::OnMenuHighlight(wxMenuEvent& WXUNUSED(event))
{
	// This prevents the default behavior of showing unhelpful help text in the status bar when the user navigates the menu.
}

void Window::OnFileOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog* file_dialog = new wxFileDialog(this, "Open File", "", "", "MIDI Files (*.mid)|*.mid|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (file_dialog->ShowModal() == wxID_OK)
	{
		if (this->midi_file != NULL) MidiFile_free(this->midi_file);
		this->midi_file = MidiFile_load((char*)(file_dialog->GetPath().ToStdString().c_str()));
		this->PrepareRows();
	}

	delete file_dialog;
}

void Window::OnExit(wxCommandEvent& WXUNUSED(event))
{
	this->Close(true);
}

void Window::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("Seqer\na MIDI sequencer\nby Div Slomin", "About", wxOK);
}

void Window::PrepareRows()
{
	this->rows.clear();

	if (this->midi_file != NULL)
	{
		int last_step = 0;

		for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
		{
			int step = (int)(MidiFile_getBeatFromTick(this->midi_file, MidiFileEvent_getTick(midi_event)));

			while (last_step < step - 1)
			{
				this->rows.push_back(Row(last_step, NULL));
				last_step++;
			}

			this->rows.push_back(Row(step, midi_event));
			last_step = step;
		}
	}

	wxClientDC dc(this->canvas);
	this->row_height = dc.GetCharHeight();
	this->char_width = dc.GetCharWidth();
	this->canvas->SetScrollbars(0, this->row_height, 0, this->rows.size());
}

Row::Row(int step, MidiFileEvent_t event)
{
	this->step = step;
	this->event = event;
}

#if defined(__WXOSX__)
#define CANVAS_BORDER wxBORDER_DEFAULT
#else
#define CANVAS_BORDER wxBORDER_THEME
#endif

Canvas::Canvas(Window* window): wxScrolledCanvas(window, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;
	this->DisableKeyboardScrolling();
}

void Canvas::OnDraw(wxDC& dc)
{
	for (long row_number = 0; row_number < this->window->rows.size(); row_number++)
	{
		wxString text;
		text.Printf("%ld of %ld", row_number, this->window->rows.size());
		dc.DrawText(text, 0, row_number * this->window->row_height);
	}
}


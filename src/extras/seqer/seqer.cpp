
#include <algorithm>
#include <vector>
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <midifile.h>
#include "color.h"

class Application: public wxApp
{
public:
	bool OnInit();
};

class Window: public wxFrame
{
public:
	class Sequence* sequence;
	class Canvas* canvas;

	Window();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnFilter(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
};

class Sequence
{
public:
	Window* window;
	MidiFile_t midi_file;

	Sequence(Window* window);
};

class Canvas: public wxScrolledCanvas
{
public:
	Window* window;
	class EventList* event_list;
	class PianoRoll* piano_roll;
	std::vector<class Row> rows;
	std::vector<class Step> steps;

	Canvas(Window* window);
	bool Load(wxString filename);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetVisibleHeight();
	long GetFirstVisibleY();
	long GetLastVisibleY();
	long GetFirstRowNumberFromStepNumber(long step_number);
	long GetLastRowNumberFromStepNumber(long step_number);
	long GetStepNumberFromRowNumber(long row_number);
};

class EventList
{
public:
	Canvas *canvas;
	wxFont font;
	long row_height;
	long column_widths[8];

	EventList(Canvas* canvas);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetFirstVisibleRowNumber();
	long GetLastVisibleRowNumber();
	long GetLastVisiblePopulatedRowNumber();
	long GetColumnWidth(long column_number);
	long GetXFromColumnNumber(long column_number);
	long GetYFromRowNumber(long row_number);
	long GetRowNumberFromY(long y);
};

class PianoRoll
{
public:
	Canvas *canvas;
	long first_note;
	long last_note;
	long key_width;
	wxColour darker_line_color;
	wxColour lighter_line_color;
	wxColour lightest_line_color;
	wxColour white_key_color;
	wxColour black_key_color;

	PianoRoll(Canvas* canvas);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetWidth();
	long GetYFromStepNumber(double step_number);
};

class Row
{
public:
	long step_number;
	MidiFileEvent_t event;

	Row(long step_number, MidiFileEvent_t event);
};

class Step
{
public:
	long first_row_number;
	long last_row_number;

	Step(long row_number);
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
	SEQER_ID_EDIT_COLUMN_1,
	SEQER_ID_EDIT_COLUMN_2,
	SEQER_ID_EDIT_COLUMN_3,
	SEQER_ID_EDIT_COLUMN_4,
	SEQER_ID_EDIT_COLUMN_5,
	SEQER_ID_EDIT_COLUMN_6,
	SEQER_ID_EDIT_COLUMN_7,
	SEQER_ID_EDIT_COLUMN_8,
	SEQER_ID_FILTER,
	SEQER_ID_ZOOM,
	SEQER_ID_PIANO_ROLL,
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

	if (this->argc > 1)
	{
		window->canvas->Load(this->argv[1]);
	}

	return true;
}

Window::Window(): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	wxMenuBar* menu_bar = new wxMenuBar(); this->SetMenuBar(menu_bar); this->Connect(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(Window::OnMenuHighlight));
		wxMenu* file_menu = new wxMenu(); menu_bar->Append(file_menu, "&File");
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O"); this->Connect(wxID_OPEN, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnFileOpen));
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...");
			file_menu->Append(wxID_REVERT, "&Revert");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W"); this->Connect(wxID_CLOSE, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnClose));
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
			edit_menu->AppendSeparator();
			wxMenu* edit_column_menu = new wxMenu(); edit_menu->AppendSubMenu(edit_column_menu, "&Columns");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_1, "Column &1\tCtrl+1");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_2, "Column &2\tCtrl+2");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_3, "Column &3\tCtrl+3");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_4, "Column &4\tCtrl+4");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_5, "Column &5\tCtrl+5");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_6, "Column &6\tCtrl+6");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_7, "Column &7\tCtrl+7");
				edit_column_menu->Append(SEQER_ID_EDIT_COLUMN_8, "Column &8\tCtrl+8");
		wxMenu* view_menu = new wxMenu(); menu_bar->Append(view_menu, "&View");
			view_menu->Append(SEQER_ID_FILTER, "&Filter...\tCtrl+Shift+F"); this->Connect(SEQER_ID_FILTER, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnFilter));
			view_menu->AppendSeparator();
			view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++");
			view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-");
			view_menu->Append(SEQER_ID_ZOOM, "Zoo&m...\tCtrl+Shift+M");
			view_menu->AppendSeparator();
			view_menu->Append(SEQER_ID_PIANO_ROLL, "&Piano Roll...");
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
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go to &Marker...\tCtrl+M");
			transport_menu->AppendSeparator();
			transport_menu->Append(SEQER_ID_PORTS, "P&orts...\tCtrl+Shift+O");
		wxMenu* tools_menu = new wxMenu(); menu_bar->Append(tools_menu, "&Tools");
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+Q");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
		wxMenu* help_menu = new wxMenu(); menu_bar->Append(help_menu, "&Help");
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About"); this->Connect(wxID_ABOUT, wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnAbout));

	this->sequence = new Sequence(this);
	this->canvas = new Canvas(this);
	this->CreateStatusBar();
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
		if (!this->canvas->Load(file_dialog->GetPath()))
		{
			wxMessageBox("Cannot open the specified MIDI file.", "Error", wxOK | wxICON_ERROR);
		}
	}

	delete file_dialog;
}

void Window::OnClose(wxCommandEvent& WXUNUSED(event))
{
	this->Close(true);
}

void Window::OnFilter(wxCommandEvent& WXUNUSED(event))
{
	wxDialog* dialog = new wxDialog(NULL, wxID_ANY, "Filter", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	dialog->SetSizer(outer_sizer);

	wxBoxSizer* controls_sizer = new wxBoxSizer(wxHORIZONTAL);
	outer_sizer->Add(controls_sizer, wxSizerFlags(1).Expand().Border());

	wxBoxSizer* event_type_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(event_type_sizer, wxSizerFlags(1).Expand());
	event_type_sizer->Add(new wxStaticText(dialog, wxID_ANY, "Event type"), wxSizerFlags(0).Center());
	wxListBox* event_type_list_box = new wxListBox(dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
	event_type_sizer->Add(event_type_list_box, wxSizerFlags(1).Expand().Border(wxTOP));
	event_type_list_box->Append("Note");
	event_type_list_box->Append("Control change");
	event_type_list_box->Append("Program change");
	event_type_list_box->Append("Aftertouch");
	event_type_list_box->Append("Pitch bend");
	event_type_list_box->Append("System exclusive");
	event_type_list_box->Append("Text");
	event_type_list_box->Append("Lyric");
	event_type_list_box->Append("Marker");
	event_type_list_box->Append("Port");
	event_type_list_box->Append("Tempo");
	event_type_list_box->Append("Time signature");
	event_type_list_box->Append("Key signature");

	wxBoxSizer* track_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(track_sizer, wxSizerFlags(1).Expand());
	track_sizer->Add(new wxStaticText(dialog, wxID_ANY, "Track"), wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT));
	wxListBox* track_list_box = new wxListBox(dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
	track_sizer->Add(track_list_box, wxSizerFlags(1).Expand().Border(wxTOP | wxLEFT | wxRIGHT));
	for (int i = 1; i <= MidiFile_getNumberOfTracks(this->sequence->midi_file); i++) track_list_box->Append(wxString::Format("%d", i));

	wxBoxSizer* channel_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(channel_sizer, wxSizerFlags(1).Expand());
	channel_sizer->Add(new wxStaticText(dialog, wxID_ANY, "Channel"), wxSizerFlags(0).Center());
	wxListBox* channel_list_box = new wxListBox(dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
	channel_sizer->Add(channel_list_box, wxSizerFlags(1).Expand().Border(wxTOP));
	for (int i = 1; i <= 16; i++) channel_list_box->Append(wxString::Format("%d", i));

	wxSizer* button_sizer = dialog->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags(0).Border());

	outer_sizer->Fit(dialog);

	if (dialog->ShowModal() == wxID_OK)
	{
	}

	dialog->Destroy();
}

void Window::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("Seqer\na MIDI sequencer\nby Div Slomin", "About", wxOK);
}

Sequence::Sequence(Window* window)
{
	this->window = window;
	this->midi_file = NULL;
}

#if defined(__WXMSW__)
#define CANVAS_BORDER wxBORDER_THEME
#else
#define CANVAS_BORDER wxBORDER_DEFAULT
#endif

Canvas::Canvas(Window* window): wxScrolledCanvas(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;
	this->event_list = new EventList(this);
	this->piano_roll = new PianoRoll(this);
	this->DisableKeyboardScrolling();
	this->SetBackgroundColour(*wxWHITE);
	this->Prepare();
}

bool Canvas::Load(wxString filename)
{ 
	MidiFile_t new_midi_file = MidiFile_load((char *)(filename.ToStdString().c_str()));
	if (new_midi_file == NULL) return false;
	if (this->window->sequence->midi_file != NULL) MidiFile_free(this->window->sequence->midi_file);
	this->window->sequence->midi_file = new_midi_file;
	this->Prepare();
	return true;
}

void Canvas::Prepare()
{
	this->rows.clear();
	this->steps.clear();

	if (this->window->sequence->midi_file != NULL)
	{
		long last_step_number = -1;

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->window->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			// TODO: real filter logic

			if (!MidiFileEvent_isNoteEndEvent(event))
			{
				long step_number = (long)(MidiFile_getBeatFromTick(this->window->sequence->midi_file, MidiFileEvent_getTick(event)));

				while (last_step_number < step_number - 1)
				{
					last_step_number++;
					this->rows.push_back(Row(last_step_number, NULL));
					this->steps.push_back(Step(this->rows.size() - 1));
				}

				this->rows.push_back(Row(step_number, event));

				if (step_number == last_step_number)
				{
					this->steps[this->steps.size() - 1].last_row_number++;
				}
				else
				{
					this->steps.push_back(Step(this->rows.size() - 1));
				}

				last_step_number = step_number;
			}
		}
	}

	this->event_list->Prepare();
	this->piano_roll->Prepare();
}

void Canvas::OnDraw(wxDC& dc)
{
	this->event_list->OnDraw(dc);
	this->piano_roll->OnDraw(dc);
}

long Canvas::GetVisibleWidth()
{
	return this->GetClientSize().GetWidth();
}

long Canvas::GetVisibleHeight()
{
	return this->GetClientSize().GetHeight();
}

long Canvas::GetFirstVisibleY()
{
	return this->event_list->GetYFromRowNumber(this->event_list->GetFirstVisibleRowNumber());
}

long Canvas::GetLastVisibleY()
{
	return this->GetFirstVisibleY() + this->GetVisibleHeight();
}

long Canvas::GetFirstRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number].first_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1].last_row_number + step_number - this->steps.size() + 1;
	}
}

long Canvas::GetLastRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number].last_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1].last_row_number + step_number - this->steps.size() + 1;
	}
}

long Canvas::GetStepNumberFromRowNumber(long row_number)
{
	if (this->rows.size() == 0)
	{
		return row_number;
	}
	else if (row_number < this->rows.size())
	{
		return this->rows[row_number].step_number;
	}
	else
	{
		return this->rows[this->rows.size() - 1].step_number + row_number - this->rows.size() + 1;
	}
}

EventList::EventList(Canvas* canvas)
{
	this->canvas = canvas;

#if defined(__WXOSX__)
	this->font = wxFont(wxFontInfo(10).FaceName("Lucida Grande"));
#else
	this->font = *wxNORMAL_FONT;
#endif
}

void EventList::Prepare()
{
	wxClientDC dc(this->canvas);
	dc.SetFont(this->font);
	this->row_height = dc.GetCharHeight();

	this->column_widths[0] = dc.GetTextExtent("Marker#").GetWidth();
	this->column_widths[1] = dc.GetTextExtent("000:0.000#").GetWidth();
	this->column_widths[2] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[3] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[4] = dc.GetTextExtent("C#0#").GetWidth();
	this->column_widths[5] = dc.GetTextExtent("000#").GetWidth();
	this->column_widths[6] = dc.GetTextExtent("000:0.000#").GetWidth();
	this->column_widths[7] = dc.GetTextExtent("000#").GetWidth();

	this->canvas->SetScrollbars(0, this->row_height, 0, this->canvas->rows.size());
}

void EventList::OnDraw(wxDC& dc)
{
	long first_y = this->canvas->GetFirstVisibleY();
	long last_y = this->canvas->GetLastVisibleY();
	long first_step_number = this->canvas->GetStepNumberFromRowNumber(this->GetRowNumberFromY(first_y));
	long last_step_number = this->canvas->GetStepNumberFromRowNumber(this->GetRowNumberFromY(last_y));
	long first_row_number = this->GetFirstVisibleRowNumber();
	long last_populated_row_number = this->GetLastVisiblePopulatedRowNumber();
	long piano_roll_width = this->canvas->piano_roll->GetWidth();
	long width = this->GetVisibleWidth();

	dc.SetPen(wxPen(this->canvas->piano_roll->lightest_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->canvas->piano_roll->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(piano_roll_width, y, piano_roll_width + width, y);
	}

	dc.SetFont(this->font);
	const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

	for (long row_number = first_row_number; row_number <= last_populated_row_number; row_number++)
	{
		Row row = this->canvas->rows[row_number];

		if (row.event == NULL)
		{
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFile_getTickFromBeat(this->canvas->window->sequence->midi_file, row.step_number)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isNoteStartEvent(row.event))
		{
			dc.DrawText("Note", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileNoteStartEvent_getChannel(row.event) + 1), this->GetXFromColumnNumber(3), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%s%d", note_names[MidiFileNoteStartEvent_getNote(row.event) % 12], (MidiFileNoteStartEvent_getNote(row.event) / 12) - 1), this->GetXFromColumnNumber(4), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileNoteStartEvent_getVelocity(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(row.event))), this->GetXFromColumnNumber(6), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(row.event))), this->GetXFromColumnNumber(7), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE)
		{
			dc.DrawText("Ctrl", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileControlChangeEvent_getChannel(row.event) + 1), this->GetXFromColumnNumber(3), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileControlChangeEvent_getNumber(row.event) + 1), this->GetXFromColumnNumber(4), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileControlChangeEvent_getValue(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE)
		{
			dc.DrawText("Prog", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileProgramChangeEvent_getChannel(row.event) + 1), this->GetXFromColumnNumber(3), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileProgramChangeEvent_getNumber(row.event) + 1), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
		{
			dc.DrawText("Touch", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileKeyPressureEvent_getChannel(row.event) + 1), this->GetXFromColumnNumber(3), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%s%d", note_names[MidiFileKeyPressureEvent_getNote(row.event) % 12], (MidiFileKeyPressureEvent_getNote(row.event) / 12) - 1), this->GetXFromColumnNumber(4), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileKeyPressureEvent_getAmount(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE)
		{
			dc.DrawText("Touch", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileChannelPressureEvent_getChannel(row.event) + 1), this->GetXFromColumnNumber(3), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileChannelPressureEvent_getAmount(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL)
		{
			dc.DrawText("Bend", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFilePitchWheelEvent_getChannel(row.event) + 1), this->GetXFromColumnNumber(3), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFilePitchWheelEvent_getValue(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_getType(row.event) == MIDI_FILE_EVENT_TYPE_SYSEX)
		{
			dc.DrawText("Sysex", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			// TODO
		}
		else if (MidiFileEvent_isTextEvent(row.event))
		{
			dc.DrawText("Text", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFileTextEvent_getText(row.event), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isLyricEvent(row.event))
		{
			dc.DrawText("Lyric", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFileLyricEvent_getText(row.event), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isMarkerEvent(row.event))
		{
			dc.DrawText("Marker", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFileMarkerEvent_getText(row.event), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isPortEvent(row.event))
		{
			dc.DrawText("Port", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFilePortEvent_getName(row.event), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isTempoEvent(row.event))
		{
			dc.DrawText("Tempo", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isTimeSignatureEvent(row.event))
		{
			dc.DrawText("Time", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(row.event), MidiFileTimeSignatureEvent_getDenominator(row.event)), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else if (MidiFileEvent_isKeySignatureEvent(row.event))
		{
			dc.DrawText("Key", this->GetXFromColumnNumber(0) + 1, this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
			dc.DrawText(wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event))), this->GetXFromColumnNumber(2), this->GetYFromRowNumber(row_number) + 1);
			const char* sharp_keys[] = {"C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#", "E#", "B#"};
			const char* flat_keys[] = {"C", "F", "Bb", "Eb", "Ab", "Db", "Gb", "Cb", "Fb"};
			dc.DrawText(wxString::Format("%s%s", MidiFileKeySignatureEvent_isFlat(row.event) ? flat_keys[MidiFileKeySignatureEvent_getNumber(row.event)] : sharp_keys[MidiFileKeySignatureEvent_getNumber(row.event)], MidiFileKeySignatureEvent_isMinor(row.event) ? "m" : ""), this->GetXFromColumnNumber(5), this->GetYFromRowNumber(row_number) + 1);
		}
		else
		{
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
		}
	}
}

long EventList::GetVisibleWidth()
{
	return this->canvas->GetVisibleWidth() - this->canvas->piano_roll->GetWidth();
}

long EventList::GetFirstVisibleRowNumber()
{
	return this->canvas->GetViewStart().y;
}

long EventList::GetLastVisibleRowNumber()
{
	return this->GetFirstVisibleRowNumber() + this->GetRowNumberFromY(this->canvas->GetVisibleHeight()) + 1;
}

long EventList::GetLastVisiblePopulatedRowNumber()
{
	return std::min(this->GetLastVisibleRowNumber(), (long)(this->canvas->rows.size() - 1));
}

long EventList::GetColumnWidth(long column_number)
{
	return this->column_widths[column_number];
}

long EventList::GetXFromColumnNumber(long column_number)
{
	long x = this->canvas->piano_roll->GetWidth();
	while (--column_number >= 0) x += this->column_widths[column_number];
	return x;
}

long EventList::GetYFromRowNumber(long row_number)
{
	return row_number * this->row_height;
}

long EventList::GetRowNumberFromY(long y)
{
	return y / this->row_height;
}

PianoRoll::PianoRoll(Canvas* canvas)
{
	this->canvas = canvas;
	this->first_note = 21; // default to standard piano range
	this->last_note = 108;
	this->key_width = 3;
}

void PianoRoll::Prepare()
{
	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->darker_line_color = ColorShade(button_color, 205 * 100 / 255);
	this->lighter_line_color = ColorShade(button_color, 215 * 100 / 255);
	this->lightest_line_color = ColorShade(button_color, 218 * 100 / 255);
	this->white_key_color = *wxWHITE;
	this->black_key_color = ColorShade(button_color, 230 * 100 / 255);
}

void PianoRoll::OnDraw(wxDC& dc)
{
	wxPen pens[] = {wxPen(this->darker_line_color), wxPen(this->lighter_line_color)};
	wxBrush brushes[] = {wxBrush(this->white_key_color), wxBrush(this->black_key_color)};
	long key_pens[] = {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1};
	long key_brushes[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
	long first_y = this->canvas->GetFirstVisibleY();
	long last_y = this->canvas->GetLastVisibleY();
	long width = this->GetWidth();
	long height = this->canvas->GetVisibleHeight();

	for (long note = this->first_note; note <= this->last_note; note++)
	{
		dc.SetPen(pens[key_pens[note % 12]]);
		dc.SetBrush(brushes[key_brushes[note % 12]]);
		dc.DrawRectangle((note - this->first_note) * this->key_width - 1, first_y - 1, this->key_width + 1, height + 2);
	}

	long first_step_number = this->canvas->GetStepNumberFromRowNumber(this->canvas->event_list->GetRowNumberFromY(first_y));
	long last_step_number = this->canvas->GetStepNumberFromRowNumber(this->canvas->event_list->GetRowNumberFromY(last_y));
	dc.SetPen(wxPen(this->lightest_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(0, y, width, y);
	}

	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxWHITE_BRUSH);

	for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->canvas->window->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isNoteStartEvent(event))
		{
			double event_step_number = MidiFile_getBeatFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event));
			long event_y = this->GetYFromStepNumber(event_step_number);

			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
			double end_event_step_number = MidiFile_getBeatFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(end_event));
			long end_event_y = this->GetYFromStepNumber(end_event_step_number);

			if ((event_y <= last_y) && (end_event_y >= first_y))
			{
				dc.DrawRectangle((MidiFileNoteOnEvent_getNote(event) - this->first_note) * this->key_width - 1, event_y, this->key_width + 1, end_event_y - event_y);
			}
		}
	}
}

long PianoRoll::GetWidth()
{
	return this->key_width * (this->last_note - this->first_note + 1);
}

long PianoRoll::GetYFromStepNumber(double step_number)
{
	long step_first_y = this->canvas->event_list->GetYFromRowNumber(this->canvas->GetFirstRowNumberFromStepNumber((long)(step_number)));
	long step_last_y = this->canvas->event_list->GetYFromRowNumber(this->canvas->GetLastRowNumberFromStepNumber((long)(step_number)) + 1);
	return step_first_y + (long)((step_last_y - step_first_y) * (step_number - (long)(step_number)));
}

Row::Row(long step_number, MidiFileEvent_t event)
{
	this->step_number = step_number;
	this->event = event;
}

Step::Step(long row_number)
{
	this->first_row_number = row_number;
	this->last_row_number = row_number;
}


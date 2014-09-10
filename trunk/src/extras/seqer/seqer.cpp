
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
	class Canvas* canvas;

	Window();
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
};

class Canvas: public wxScrolledCanvas
{
public:
	class Window* window;
	wxFont font;
	long first_note;
	long last_note;
	long key_width;
	MidiFile_t midi_file;
	std::vector<class Row> rows;
	long row_height;
	long char_width;
	wxColour darker_line_color;
	wxColour lighter_line_color;
	wxColour white_key_color;
	wxColour black_key_color;

	Canvas(Window* window);
	bool Load(char* filename);
	void Prepare();
	void OnDraw(wxDC& dc);
};

class Row
{
public:
	long step;
	MidiFileEvent_t event;

	Row(long step, MidiFileEvent_t event);
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
	return true;
}

Window::Window(): wxFrame((wxFrame*)(NULL), -1, "Seqer", wxDefaultPosition, wxSize(640, 480))
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
			view_menu->Append(SEQER_ID_FILTER, "&Filter...\tCtrl+Shift+F");
			view_menu->AppendSeparator();
			view_menu->Append(SEQER_ID_ZOOM, "Zoo&m...\tCtrl+Shift+M");
			view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++");
			view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-");
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
			transport_menu->Append(SEQER_ID_GO_TO_MARKER, "Go To &Marker...\tCtrl+M");
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
		if (!this->canvas->Load((char*)(file_dialog->GetPath().ToStdString().c_str())))
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

void Window::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox("Seqer\na MIDI sequencer\nby Div Slomin", "About", wxOK);
}

#if defined(__WXMSW__)
#define CANVAS_BORDER wxBORDER_THEME
#else
#define CANVAS_BORDER wxBORDER_DEFAULT
#endif

Canvas::Canvas(Window* window): wxScrolledCanvas(window, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;

#if defined(__WXOSX__)
	this->font = wxFont(wxFontInfo(10).FaceName("Lucida Grande"));
#else
	this->font = *wxNORMAL_FONT;
#endif

	this->first_note = 21; // default to standard piano range
	this->last_note = 108;
	this->key_width = 3;
	this->midi_file = NULL;
	this->DisableKeyboardScrolling();
	this->Prepare();
}

bool Canvas::Load(char* filename)
{ 
	MidiFile_t new_midi_file = MidiFile_load(filename);
	if (new_midi_file == NULL) return false;
	if (this->midi_file != NULL) MidiFile_free(this->midi_file);
	this->midi_file = new_midi_file;
	this->Prepare();
	return true;
}

void Canvas::Prepare()
{
	this->rows.clear();

	if (this->midi_file != NULL)
	{
		long last_step = 0;

		for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
		{
			long step = (long)(MidiFile_getBeatFromTick(this->midi_file, MidiFileEvent_getTick(midi_event)));

			while (last_step < step - 1)
			{
				this->rows.push_back(Row(last_step, NULL));
				last_step++;
			}

			this->rows.push_back(Row(step, midi_event));
			last_step = step;
		}
	}

	wxClientDC dc(this);
	dc.SetFont(this->font);
	this->row_height = dc.GetCharHeight();
	this->char_width = dc.GetCharWidth();
	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->darker_line_color = ColorShade(button_color, 205 * 100 / 255);
	this->lighter_line_color = ColorShade(button_color, 215 * 100 / 255);
	this->white_key_color = *wxWHITE;
	this->black_key_color = ColorShade(button_color, 230 * 100 / 255);
	this->SetScrollbars(0, this->row_height, 0, this->rows.size());
}

void Canvas::OnDraw(wxDC& dc)
{
	wxPen pens[] = {wxPen(this->darker_line_color), wxPen(this->lighter_line_color)};
	wxBrush brushes[] = {wxBrush(this->white_key_color), wxBrush(this->black_key_color)};
	long key_pens[] = {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1};
	long key_brushes[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};

	long width = this->GetClientSize().GetWidth();
	long height = this->GetClientSize().GetHeight();
	long y = this->GetViewStart().y * this->row_height;

	for (long note = this->first_note; note <= this->last_note; note++)
	{
		dc.SetPen(pens[key_pens[note % 12]]);
		dc.SetBrush(brushes[key_brushes[note % 12]]);
		dc.DrawRectangle((note - this->first_note) * this->key_width - 1, y - 1, this->key_width + 1, height + 2);
	}

	dc.SetFont(this->font);

	long first_row_number = this->GetViewStart().y;
	long last_row_number = std::min((long)(first_row_number + (this->GetClientSize().GetHeight() / this->row_height)), (long)(this->rows.size()));

	for (long row_number = first_row_number; row_number < last_row_number; row_number++)
	{
		Row row = this->rows[row_number];
		wxString text;

		if (row.event == NULL)
		{
			text.Printf("step %ld", row.step);
		}
		else
		{
			switch(MidiFileEvent_getType(row.event))
			{
				case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
				{
					text.Printf("step %ld, note off, tick %ld, trk %d, ch %d, note %d, vel %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileNoteOffEvent_getChannel(row.event), MidiFileNoteOffEvent_getNote(row.event), MidiFileNoteOffEvent_getVelocity(row.event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NOTE_ON:
				{
					text.Printf("step %ld, note on, tick %ld, trk %d, ch %d, note %d, vel %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileNoteOnEvent_getChannel(row.event), MidiFileNoteOnEvent_getNote(row.event), MidiFileNoteOnEvent_getVelocity(row.event));
					dc.SetPen(*wxBLACK_PEN);
					dc.SetBrush(wxNullBrush);
					dc.DrawRectangle((MidiFileNoteOnEvent_getNote(row.event) - this->first_note) * this->key_width - 1, row_number * this->row_height, this->key_width + 1, this->row_height);
					break;
				}
				case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
				{
					text.Printf("step %ld, key pressure, tick %ld, trk %d, ch %d, note %d, amt %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileKeyPressureEvent_getChannel(row.event), MidiFileKeyPressureEvent_getNote(row.event), MidiFileKeyPressureEvent_getAmount(row.event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
				{
					text.Printf("step %ld, control change, tick %ld, trk %d, ch %d, num %d, val %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileControlChangeEvent_getChannel(row.event), MidiFileControlChangeEvent_getNumber(row.event), MidiFileControlChangeEvent_getValue(row.event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
				{
					text.Printf("step %ld, program change, tick %ld, trk %d, ch %d, num %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileProgramChangeEvent_getChannel(row.event), MidiFileProgramChangeEvent_getNumber(row.event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
				{
					text.Printf("step %ld, channel pressure, tick %ld, trk %d, ch %d, amt %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileChannelPressureEvent_getChannel(row.event), MidiFileChannelPressureEvent_getAmount(row.event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
				{
					text.Printf("step %ld, pitch wheel, tick %ld, trk %d, ch %d, val %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFilePitchWheelEvent_getChannel(row.event), MidiFilePitchWheelEvent_getValue(row.event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_SYSEX:
				{
					text.Printf("step %ld, sysex, tick %ld, trk %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_META:
				{
					text.Printf("step %ld, meta, tick %ld, trk %d, num %d", row.step, MidiFileEvent_getTick(row.event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(row.event)), MidiFileMetaEvent_getNumber(row.event));
					break;
				}
				default:
				{
					text.Empty();
					break;
				}
			}
		}

		dc.DrawText(text, this->key_width * (this->last_note - this->first_note + 1) + 1, row_number * this->row_height);
	}
}

Row::Row(long step, MidiFileEvent_t event)
{
	this->step = step;
	this->event = event;
}


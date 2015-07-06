
#include <algorithm>
#include <vector>
#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/fontpicker.h>
#include <midifile.h>
#include "color.h"

class Application: public wxApp
{
public:
	double default_steps_per_beat;
	wxFont default_event_list_font;
	long default_piano_roll_first_note;
	long default_piano_roll_last_note;
	long default_piano_roll_key_width;

	bool OnInit();
};

class Window: public wxFrame
{
public:
	Application* application;
	class Sequence* sequence;
	class Canvas* canvas;

	Window(Application* application);
	void OnMenuHighlight(wxMenuEvent& event);
	void OnFileOpen(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnFilter(wxCommandEvent& event);
	void OnSettings(wxCommandEvent& event);
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
#ifdef NEW_STEP_ZOOM

	struct
	{
		StepType type;

		union
		{
			struct
			{
				int numerator;
				int denominator;
				int amount;
			}
			steps_per_measure;

			struct
			{
				int amount;
			}
			measures_per_step;

			struct
			{
				double amount;
			}
			seconds_per_step;
		}
		u;
	}
	step_config;

#else
	double steps_per_beat;
#endif
	std::vector<int> filtered_event_types;
	std::vector<int> filtered_tracks;
	std::vector<int> filtered_channels;

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
	long GetStepNumberFromTick(long tick);
	double GetFractionalStepNumberFromTick(long tick);
	bool Filter(MidiFileEvent_t event);
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
	wxColour shadow_color;

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

class FilterDialog: public wxDialog
{
public:
	Window* window;
	wxListBox* event_type_list_box;
	wxListBox* track_list_box;
	wxListBox* channel_list_box;

	FilterDialog(Window* window);
	void OnEventTypeLabelClick(wxMouseEvent& event);
	void OnTrackLabelClick(wxMouseEvent& event);
	void OnChannelLabelClick(wxMouseEvent& event);
};

class SettingsDialog: public wxDialog
{
public:
	Window* window;
	wxFontPickerCtrl* event_list_font_picker;
	wxTextCtrl* piano_roll_first_note_text_box;
	wxTextCtrl* piano_roll_last_note_text_box;
	wxTextCtrl* piano_roll_key_width_text_box;

	SettingsDialog(Window* window);
	void OnDefaultEventListFontButtonClick(wxCommandEvent& event);
	void OnDefaultPianoRollFirstNoteButtonClick(wxCommandEvent& event);
	void OnDefaultPianoRollLastNoteButtonClick(wxCommandEvent& event);
	void OnDefaultPianoRollKeyWidthButtonClick(wxCommandEvent& event);
};

class EventType
{
public:
	virtual wxString GetName() = 0;
	virtual bool Matches(MidiFileEvent_t event) = 0;
	virtual wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number) = 0;
};

class NoteEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class ControlChangeEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class ProgramChangeEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class AftertouchEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class PitchBendEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class SystemExclusiveEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class TextEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class LyricEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class MarkerEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class PortEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class TempoEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class TimeSignatureEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class KeySignatureEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

EventType* event_types[] = {new NoteEventType(), new ControlChangeEventType(), new ProgramChangeEventType(), new AftertouchEventType(), new PitchBendEventType(), new SystemExclusiveEventType(), new TextEventType(), new LyricEventType(), new MarkerEventType(), new PortEventType(), new TempoEventType(), new TimeSignatureEventType(), new KeySignatureEventType()};

enum
{
	STEPS_PER_MEASURE,
	MEASURES_PER_STEP,
	SECONDS_PER_STEP
}
StepType;

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
	SEQER_ID_STEP_SIZE,
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
	SEQER_ID_EXTERNAL_UTILITY,
	SEQER_ID_RECORD_MACRO,
	SEQER_ID_MACROS,
	SEQER_ID_SETTINGS,
	SEQER_ID_HIGHEST
};

IMPLEMENT_APP(Application)

wxString GetNoteNameFromNumber(int note_number)
{
	const char* note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	return wxString::Format("%s%d", note_names[note_number % 12], (note_number / 12) - 1);
}

int GetNoteNumberFromName(wxString note_name)
{
	const char* note_names[] = {"C#", "C", "Db", "D#", "D", "Eb", "E", "F#", "F", "Gb", "G#", "G", "Ab", "A#", "A", "Bb", "B"};
	const int chromatics[] = {1, 0, 1, 3, 2, 3, 4, 6, 5, 6, 8, 7, 8, 10, 9, 10, 11};
	int chromatic = -1;
	int octave = -1;
	int length;

	for (int i = 0; i < (sizeof note_names / sizeof (char*)); i++)
	{
		length = strlen(note_names[i]);

		if (strncmp(note_names[i], note_name.c_str(), length) == 0)
		{
			chromatic = chromatics[i];
			break;
		}
	}

	if (chromatic < 0) return -1;
	if (sscanf(note_name.c_str() + length, "%d", &octave) != 1) return -1;
	return ((octave + 1) * 12) + chromatic;
}

int GetNoteOctave(int note_number)
{
	return (note_number / 12) - 1;
}

int SetNoteOctave(int note_number, int octave)
{
	return (note_number % 12) + ((octave + 1) * 12);
}

int GetNoteChromatic(int note_number)
{
	return note_number % 12;
}

int SetNoteChromatic(int note_number, int chromatic)
{
	return ((note_number / 12) * 12) + chromatic;
}

wxString GetKeyNameFromNumber(int key_number, bool is_minor)
{
	while (key_number < -6) key_number += 12;
	while (key_number > 6) key_number -= 12;
	const char* key_names[] = {"Gb", "Db", "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#"};
	return wxString::Format("%s%s", key_names[key_number + 6], is_minor ? "m" : "");
}

int GetChromaticFromDiatonicInKey(int diatonic, int key_number)
{
	while (key_number < -6) key_number += 12;
	while (key_number > 6) key_number -= 12;
	const int diatonic_to_chromatic[] = {0, 2, 4, 5, 7, 9, 11};
	const int key_diatonics[] = {4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3};
	const int key_chromatics[] = {6, 1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6};
	return (key_chromatics[key_number] + diatonic_to_chromatic[(7 + diatonic - key_diatonics[key_number]) % 7]) % 12;
}

/*
 * Zooming in steps per measure mode moves through a series of logical
 * divisions of a measure -- first the factors of the time signature numerator,
 * then powers of two times the time signature numerator.
 *
 * Examples:
 *
 * 4/4: 1, 2, 4, 8, 16, ...
 * 3/4: 1, 3, 6, 12, 24, ...
 * 3/8: 1, 3, 6, 12, 24, ...
 * 6/8: 1, 2, 3, 6, 12, 24, ...
 * 12/8: 1, 2, 3, 4, 6, 12, 24, ...
 * 5/8: 1, 5, 10, 20, ...
 */

int GetFinerMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	if (existing_measure_division < time_signature_numerator)
	{
		for (int finer_measure_division = existing_measure_division + 1; true; finer_measure_division++)
		{
			if (time_signature_numerator % finer_measure_division == 0) return finer_measure_division;
		}
	}
	else
	{
		for (int power_of_two = 1; true; power_of_two++)
		{
			int finer_measure_division = time_signature_numerator << power_of_two;
			if (finer_measure_division > existing_measure_division) return finer_measure_division;
		}
	}
}

int GetCoarserMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	if (existing_measure_division <= time_signature_numerator)
	{
		for (int coarser_measure_division = existing_measure_division - 1; true; coarser_measure_division--)
		{
			if (time_signature_numerator % coarser_measure_division == 0) return coarser_measure_division;
		}
	}
	else
	{
		for (int power_of_two = 1; true; power_of_two++)
		{
			int coarser_measure_division = time_signature_numerator << power_of_two;
			if (coarser_measure_division >= existing_measure_division) return coarser_measure_division >> 1;
		}
	}
}

int GetFinerOrEqualMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	int finer_measure_division = GetFinerMeasureDivision(existing_measure_division, time_signature_numerator);
	int coarser_measure_division = GetCoarserMeasureDivision(finer_measure_division, time_signature_numerator);
	return (existing_measure_division == coarser_measure_division) ? existing_measure_division : finer_measure_division;
}

int GetCoarserOrEqualMeasureDivision(int existing_measure_division, int time_signature_numerator)
{
	int coarser_measure_division = GetCoarserMeasureDivision(existing_measure_division, time_signature_numerator);
	int finer_measure_division = GetFinerMeasureDivision(coarser_measure_division, time_signature_numerator);
	return (existing_measure_division == finer_measure_division) ? existing_measure_division : coarser_measure_division;
}

int GetEquivalentMeasureDivision(int source_measure_division, int source_time_signature_numerator, int source_time_signature_denominator, int target_time_signature_numerator, int target_time_signature_denominator)
{
	if ((source_time_signature_numerator == target_time_signature_numerator) && (source_time_signature_denominator == target_time_signature_denominator)) return source_measure_division;
	double raw_target_measure_division = (double)(source_measure_division * target_time_signature_numerator * source_time_signature_denominator) / (double)(source_time_signature_numerator * target_time_signature_denominator);
	int coarser_target_measure_division = GetCoarserOrEqualMeasureDivision((int)(raw_target_measure_division), target_time_signature_numerator);
	int finer_target_measure_division = GetFinerMeasureDivision(coarser_target_measure_division, target_time_signature_numerator);
	return ((double)(finer_target_measure_division) - raw_target_measure_division < raw_target_measure_division - (double)(coarser_target_measure_division)) ? finer_target_measure_division : coarser_target_measure_division;
}

int GetFinerMeasureMultiple(int existing_measure_multiple)
{
	for (int power_of_two = 1; true; power_of_two++)
	{
		int finer_measure_multiple = 1 << power_of_two;
		if (finer_measure_multiple >= existing_measure_multiple) return finer_measure_multiple >> 1;
	}
}

int GetCoarserMeasureMultiple(int existing_measure_multiple)
{
	for (int power_of_two = 1; true; power_of_two++)
	{
		int coarser_measure_multiple = 1 << power_of_two;
		if (coarser_measure_multiple > existing_measure_multiple) return coarser_measure_multiple;
	}
}

bool Application::OnInit()
{
	this->default_steps_per_beat = 1;
#if defined(__WXOSX__)
	this->default_event_list_font = wxFont(wxFontInfo(10).FaceName("Lucida Grande"));
#else
	this->default_event_list_font = *wxNORMAL_FONT;
#endif
	this->default_piano_roll_first_note = 21; // standard piano range
	this->default_piano_roll_last_note = 108;
	this->default_piano_roll_key_width = 3;

	Window* window = new Window(this);
	this->SetTopWindow(window);
	window->Show(true);

	if (this->argc > 1)
	{
		window->canvas->Load(this->argv[1]);
	}

	return true;
}

Window::Window(Application* application): wxFrame((wxFrame*)(NULL), wxID_ANY, "Seqer", wxDefaultPosition, wxSize(640, 480))
{
	this->application = application;

	wxMenuBar* menu_bar = new wxMenuBar(); this->SetMenuBar(menu_bar); this->Bind(wxEVT_MENU_HIGHLIGHT, &Window::OnMenuHighlight, this);
		wxMenu* file_menu = new wxMenu(); menu_bar->Append(file_menu, "&File");
			file_menu->Append(wxID_NEW, "&New\tCtrl+N");
			file_menu->Append(SEQER_ID_NEW_WINDOW, "New &Window\tCtrl+Shift+N");
			file_menu->Append(wxID_OPEN, "&Open...\tCtrl+O"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnFileOpen, this, wxID_OPEN);
			file_menu->Append(wxID_SAVE, "&Save\tCtrl+S");
			file_menu->Append(wxID_SAVEAS, "Save &As...");
			file_menu->Append(wxID_REVERT, "&Revert");
			file_menu->AppendSeparator();
			file_menu->Append(wxID_CLOSE, "&Close\tCtrl+W"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnClose, this, wxID_CLOSE);
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
		wxMenu* view_menu = new wxMenu(); menu_bar->Append(view_menu, "&View");
			view_menu->Append(wxID_ZOOM_IN, "Zoom &In\tCtrl++"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnZoomIn, this, wxID_ZOOM_IN);
			view_menu->Append(wxID_ZOOM_OUT, "Zoom &Out\tCtrl+-"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnZoomOut, this, wxID_ZOOM_OUT);
			view_menu->Append(SEQER_ID_STEP_SIZE, "&Step Size...\tCtrl+Shift+S");
			view_menu->AppendSeparator();
			view_menu->Append(SEQER_ID_FILTER, "&Filter...\tCtrl+Shift+F"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnFilter, this, SEQER_ID_FILTER);
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
			tools_menu->Append(SEQER_ID_EXTERNAL_UTILITY, "External &Utility...\tCtrl+Shift+U");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_RECORD_MACRO, "&Record Macro...\tCtrl+Shift+Q");
			tools_menu->Append(SEQER_ID_MACROS, "&Macros...");
			tools_menu->AppendSeparator();
			tools_menu->Append(SEQER_ID_SETTINGS, "&Settings..."); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnSettings, this, SEQER_ID_SETTINGS);
		wxMenu* help_menu = new wxMenu(); menu_bar->Append(help_menu, "&Help");
			help_menu->Append(wxID_HELP_CONTENTS, "&User Manual");
			help_menu->Append(wxID_ABOUT, "&About"); this->Bind(wxEVT_COMMAND_MENU_SELECTED, &Window::OnAbout, this, wxID_ABOUT);

	// Make Unix work the same as Windows and Mac by ignoring whether shift is pressed for the zoom in/out keyboard shortcuts.
	wxAcceleratorEntry additional_keyboard_accelerators[] = {wxAcceleratorEntry(wxACCEL_CTRL, '_', wxID_ZOOM_OUT), wxAcceleratorEntry(wxACCEL_CTRL, '=', wxID_ZOOM_IN), wxAcceleratorEntry(wxACCEL_SHIFT, WXK_LEFT, SEQER_ID_SELECT_CURRENT)};
	this->SetAcceleratorTable(wxAcceleratorTable(sizeof additional_keyboard_accelerators / sizeof (wxAcceleratorEntry), additional_keyboard_accelerators));

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

void Window::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
#ifdef NEW_STEP_ZOOM
	switch (this->canvas->step_config.type)
	{
		case STEPS_PER_MEASURE:
		{
			int current_measure_division = GetEquivalentMeasureDivision(this->canvas->step_config.u.steps_per_measure.amount, this->canvas->step_config.u.steps_per_measure.numerator, this->canvas->step_config.u.steps_per_measure.denominator, current_time_signature_numerator, current_time_signature_denominator);
			this->canvas->step_config.u.steps_per_measure.numerator = current_time_signature_numerator;
			this->canvas->step_config.u.steps_per_measure.denominator = current_time_signature_denominator;
			this->canvas->step_config.u.steps_per_measure.amount = GetFinerMeasureDivision(current_measure_division, current_time_signature_numerator);
			break;
		}
		case MEASURES_PER_STEP:
		{
			if (this->canvas->step_config.u.measures_per_step == 1)
			{
				this->canvas->step_config.type = STEPS_PER_MEASURE;
				this->canvas->step_config.u.steps_per_measure.numerator = current_time_signature_numerator;
				this->canvas->step_config.u.steps_per_measure.denominator = current_time_signature_denominator;
				this->canvas->step_config.u.steps_per_measure.amount = GetFinerMeasureDivision(1, current_time_signature_numerator);
			}
			else
			{
				this->canvas->step_config.u.measures_per_step.amount = GetFinerMeasureMultiple(this->canvas->step_config.u.measures_per_step.amount);
			}

			break;
		}
		case SECONDS_PER_STEP:
		{
			this->canvas->step_config.u.seconds_per_step.amount /= 2;
			break;
		}
	}

#else
	this->canvas->steps_per_beat *= 2;
#endif
	this->canvas->Prepare();
}

void Window::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
#ifdef NEW_STEP_ZOOM
	switch (this->canvas->step_config.type)
	{
		case STEPS_PER_MEASURE:
		{
			int current_measure_division = GetEquivalentMeasureDivision(this->canvas->step_config.u.steps_per_measure.amount, this->canvas->step_config.u.steps_per_measure.numerator, this->canvas->step_config.u.steps_per_measure.denominator, current_time_signature_numerator, current_time_signature_denominator);

			if (current_measure_division == 1)
			{
				this->canvas->step_config.type = MEASURES_PER_STEP;
				this->canvas->step_config.u.measures_per_step.amount = 2;
			}
			else
			{
				this->canvas->step_config.u.steps_per_measure.numerator = current_time_signature_numerator;
				this->canvas->step_config.u.steps_per_measure.denominator = current_time_signature_denominator;
				this->canvas->step_config.u.steps_per_measure.amount = GetCoarserMeasureDivision(current_measure_division, current_time_signature_numerator);
			}

			break;
		}
		case MEASURES_PER_STEP:
		{
			this->canvas->step_config.u.measures_per_step.amount = GetCoarserMeasureMultiple(this->canvas->step_config.u.measures_per_step.amount);
			break;
		}
		case SECONDS_PER_STEP:
		{
			this->canvas->step_config.u.seconds_per_step.amount *= 2;
			break;
		}
	}

#else
	this->canvas->steps_per_beat /= 2;
#endif
	this->canvas->Prepare();
}

void Window::OnFilter(wxCommandEvent& WXUNUSED(event))
{
	FilterDialog* dialog = new FilterDialog(this);

	if (dialog->ShowModal() == wxID_OK)
	{
		wxArrayInt selections;

		this->canvas->filtered_event_types.clear();
		dialog->event_type_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) this->canvas->filtered_event_types.push_back(selections[i]);

		this->canvas->filtered_tracks.clear();
		dialog->track_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) this->canvas->filtered_tracks.push_back(selections[i]);

		this->canvas->filtered_channels.clear();
		dialog->channel_list_box->GetSelections(selections);
		for (int i = 0; i < selections.GetCount(); i++) this->canvas->filtered_channels.push_back(selections[i]);

		this->canvas->Prepare();
	}

	dialog->Destroy();
}

void Window::OnSettings(wxCommandEvent& WXUNUSED(event))
{
	SettingsDialog* dialog = new SettingsDialog(this);

	if (dialog->ShowModal() == wxID_OK)
	{
		this->canvas->event_list->font = dialog->event_list_font_picker->GetSelectedFont();

		long first_note = GetNoteNumberFromName(dialog->piano_roll_first_note_text_box->GetValue());
		long last_note = GetNoteNumberFromName(dialog->piano_roll_last_note_text_box->GetValue());

		if ((first_note >= 0) && (first_note < 128) && (last_note >= 0) && (last_note < 128) && (first_note <= last_note))
		{
			this->canvas->piano_roll->first_note = first_note;
			this->canvas->piano_roll->last_note = last_note;
		}

		long key_width = -1;
		dialog->piano_roll_key_width_text_box->GetValue().ToCLong(&key_width);
		if (key_width >= 0) this->canvas->piano_roll->key_width = key_width;

		this->canvas->Prepare();
		this->canvas->Refresh();
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
	this->steps_per_beat = this->window->application->default_steps_per_beat;
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
			if (!this->Filter(event)) continue;

			long step_number = this->GetStepNumberFromTick(MidiFileEvent_getTick(event));

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

	this->event_list->Prepare();
	this->piano_roll->Prepare();
	this->Refresh();
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

long Canvas::GetStepNumberFromTick(long tick)
{
	return (long)(this->GetFractionalStepNumberFromTick(tick));
}

double Canvas::GetFractionalStepNumberFromTick(long tick)
{
#ifdef NEW_STEP_ZOOM
	MidiFileTrack_t conductor_track = MidiFile_getFirstTrack(this->window->sequence->midi_file);
	float time_signature_event_beat = 0.0;
	int numerator = 4;
	int denominator = 4;
	float measure = 0.0;

	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(conductor_track); (event != NULL) && (MidiFileEvent_getTick(event) < tick); event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isTimeSignatureEvent(event))
		{
			float next_time_signature_event_beat = MidiFile_getBeatFromTick(this->window->sequence->midi_file, MidiFileEvent_getTick(event));
			measure += ((next_time_signature_event_beat - time_signature_event_beat) * denominator / 4 / numerator);
			time_signature_event_beat = next_time_signature_event_beat;
			numerator = MidiFileTimeSignatureEvent_getNumerator(event);
			denominator = MidiFileTimeSignatureEvent_getDenominator(event);
		}
	}

	measure += ((MidiFile_getBeatFromTick(this->window->sequence->midi_file, tick) - time_signature_event_beat) * denominator / 4 / numerator);
	MidiFileMeasureBeat_setMeasure(measure_beat, (long)(measure) + 1);
	MidiFileMeasureBeat_setBeat(measure_beat, ((measure - (MidiFileMeasureBeat_getMeasure(measure_beat) - (float)(1.0))) * (float)(numerator)) + (float)(1.0));
	return 0;
#else
	return MidiFile_getBeatFromTick(this->window->sequence->midi_file, tick) * this->steps_per_beat;
#endif
}

bool Canvas::Filter(MidiFileEvent_t event)
{
	if (this->filtered_event_types.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_event_types.size(); i++)
		{
			if (event_types[this->filtered_event_types[i]]->Matches(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}
	else
	{
		bool matched = false;

		for (int i = 0; i < (sizeof event_types / sizeof (EventType*)); i++)
		{
			if (event_types[i]->Matches(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	if (this->filtered_tracks.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_tracks.size(); i++)
		{
			if (this->filtered_tracks[i] == MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	if ((this->filtered_channels.size() > 0) && MidiFileEvent_isVoiceEvent(event))
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_channels.size(); i++)
		{
			if (this->filtered_channels[i] == MidiFileVoiceEvent_getChannel(event))
			{
				matched = true;
				break;
			}
		}

		if (!matched) return false;
	}

	return true;
}

EventList::EventList(Canvas* canvas)
{
	this->canvas = canvas;
	this->font = this->canvas->window->application->default_event_list_font;
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

	for (long row_number = first_row_number; row_number <= last_populated_row_number; row_number++)
	{
		Row row = this->canvas->rows[row_number];

		if (row.event == NULL)
		{
			dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFile_getTickFromBeat(this->canvas->window->sequence->midi_file, row.step_number)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
		}
		else
		{
			bool matched = false;

			for (int event_type_number = 0; event_type_number < (sizeof event_types / sizeof (EventType*)); event_type_number++)
			{
				if (event_types[event_type_number]->Matches(row.event))
				{
					matched = true;

					for (int column_number = 0; column_number < 8; column_number++)
					{
						wxString column_text = event_types[event_type_number]->GetEventListColumnText(this, row.event, column_number);
						if (!column_text.IsEmpty()) dc.DrawText(column_text, this->GetXFromColumnNumber(column_number) + (column_number == 0 ? 1 : 0), this->GetYFromRowNumber(row_number) + 1);
					}

					break;
				}
			}

			if (!matched) dc.DrawText(MidiFile_getMeasureBeatStringFromTick(this->canvas->window->sequence->midi_file, MidiFileEvent_getTick(row.event)), this->GetXFromColumnNumber(1), this->GetYFromRowNumber(row_number) + 1);
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
	this->first_note = this->canvas->window->application->default_piano_roll_first_note;
	this->last_note = this->canvas->window->application->default_piano_roll_last_note;
	this->key_width = this->canvas->window->application->default_piano_roll_key_width;
}

void PianoRoll::Prepare()
{
	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->darker_line_color = ColorShade(button_color, 205 * 100 / 255);
	this->lighter_line_color = ColorShade(button_color, 215 * 100 / 255);
	this->lightest_line_color = ColorShade(button_color, 218 * 100 / 255);
	this->white_key_color = *wxWHITE;
	this->black_key_color = ColorShade(button_color, 230 * 100 / 255);
	this->shadow_color = ColorShade(button_color, 192 * 100 / 255);
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

	const wxPen pass_pens[] = {wxPen(this->shadow_color), *wxBLACK_PEN};
	const wxBrush pass_brushes[] = {wxBrush(this->shadow_color), *wxWHITE_BRUSH};
	const long pass_offsets[] = {1, 0};

	for (int pass = 0; pass < 2; pass++)
	{
		dc.SetPen(pass_pens[pass]);
		dc.SetBrush(pass_brushes[pass]);

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->canvas->window->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			if (MidiFileEvent_isNoteStartEvent(event) && this->canvas->Filter(event))
			{
				double event_step_number = this->canvas->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(event));
				long event_y = this->GetYFromStepNumber(event_step_number);

				MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);
				double end_event_step_number = this->canvas->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(end_event));
				long end_event_y = this->GetYFromStepNumber(end_event_step_number);

				if ((event_y <= last_y) && (end_event_y >= first_y))
				{
					dc.DrawRectangle((MidiFileNoteOnEvent_getNote(event) - this->first_note) * this->key_width - 1 + pass_offsets[pass], event_y + pass_offsets[pass], this->key_width + 1, end_event_y - event_y);
				}
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

FilterDialog::FilterDialog(Window* window): wxDialog(NULL, wxID_ANY, "Filter", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	this->window = window;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	wxBoxSizer* controls_sizer = new wxBoxSizer(wxHORIZONTAL);
	outer_sizer->Add(controls_sizer, wxSizerFlags().Proportion(1).Expand().Border());

	wxBoxSizer* event_type_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(event_type_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* event_type_label = new wxStaticText(this, wxID_ANY, "Event type");
	event_type_sizer->Add(event_type_label, wxSizerFlags().Align(wxALIGN_CENTER));
	event_type_label->Bind(wxEVT_LEFT_DOWN, &FilterDialog::OnEventTypeLabelClick, this);
	event_type_label->Bind(wxEVT_LEFT_DCLICK, &FilterDialog::OnEventTypeLabelClick, this);

	wxArrayString event_type_names;
	for (int i = 0; i < (sizeof event_types / sizeof (EventType*)); i++) event_type_names.Add(event_types[i]->GetName());
	this->event_type_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, event_type_names, wxLB_MULTIPLE);
	for (int i = 0; i < this->window->canvas->filtered_event_types.size(); i++) this->event_type_list_box->SetSelection(this->window->canvas->filtered_event_types[i]);
	event_type_sizer->Add(this->event_type_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP));

	wxBoxSizer* track_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(track_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* track_label = new wxStaticText(this, wxID_ANY, "Track");
	track_sizer->Add(track_label, wxSizerFlags().Align(wxALIGN_CENTER).Border(wxLEFT | wxRIGHT));
	track_label->Bind(wxEVT_LEFT_DOWN, &FilterDialog::OnTrackLabelClick, this);
	track_label->Bind(wxEVT_LEFT_DCLICK, &FilterDialog::OnTrackLabelClick, this);

	wxArrayString tracks;
	for (int i = 0; i < MidiFile_getNumberOfTracks(this->window->sequence->midi_file); i++) tracks.Add(wxString::Format("%d", i));
	this->track_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, tracks, wxLB_MULTIPLE);
	for (int i = 0; i < this->window->canvas->filtered_tracks.size(); i++) this->track_list_box->SetSelection(this->window->canvas->filtered_tracks[i]);
	track_sizer->Add(this->track_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP | wxLEFT | wxRIGHT));

	wxBoxSizer* channel_sizer = new wxBoxSizer(wxVERTICAL);
	controls_sizer->Add(channel_sizer, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* channel_label = new wxStaticText(this, wxID_ANY, "Channel");
	channel_sizer->Add(channel_label, wxSizerFlags().Align(wxALIGN_CENTER));
	channel_label->Bind(wxEVT_LEFT_DOWN, &FilterDialog::OnChannelLabelClick, this);
	channel_label->Bind(wxEVT_LEFT_DCLICK, &FilterDialog::OnChannelLabelClick, this);

	wxArrayString channels;
	for (int i = 1; i <= 16; i++) channels.Add(wxString::Format("%d", i));
	this->channel_list_box = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, channels, wxLB_MULTIPLE);
	for (int i = 0; i < this->window->canvas->filtered_channels.size(); i++) this->channel_list_box->SetSelection(this->window->canvas->filtered_channels[i]);
	channel_sizer->Add(this->channel_list_box, wxSizerFlags().Proportion(1).Expand().Border(wxTOP));

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);
}

void FilterDialog::OnEventTypeLabelClick(wxMouseEvent& event)
{
	int count = this->event_type_list_box->GetCount();
	wxArrayInt selections;

	if (this->event_type_list_box->GetSelections(selections) == count)
	{
		for (int i = 0; i < count; i++)
		{
			this->event_type_list_box->Deselect(i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			this->event_type_list_box->SetSelection(i);
		}
	}

	this->event_type_list_box->SetFocus();
	event.Skip();
}

void FilterDialog::OnTrackLabelClick(wxMouseEvent& event)
{
	int count = this->track_list_box->GetCount();
	wxArrayInt selections;

	if (this->track_list_box->GetSelections(selections) == count)
	{
		for (int i = 0; i < count; i++)
		{
			this->track_list_box->Deselect(i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			this->track_list_box->SetSelection(i);
		}
	}

	this->track_list_box->SetFocus();
	event.Skip();
}

void FilterDialog::OnChannelLabelClick(wxMouseEvent& event)
{
	int count = this->channel_list_box->GetCount();
	wxArrayInt selections;

	if (this->channel_list_box->GetSelections(selections) == count)
	{
		for (int i = 0; i < count; i++)
		{
			this->channel_list_box->Deselect(i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			this->channel_list_box->SetSelection(i);
		}
	}

	this->channel_list_box->SetFocus();
	event.Skip();
}

SettingsDialog::SettingsDialog(Window* window): wxDialog(NULL, wxID_ANY, "Settings")
{
	this->window = window;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	wxStaticBoxSizer* event_list_settings_section = new wxStaticBoxSizer(wxVERTICAL, this, "Event list");
	outer_sizer->Add(event_list_settings_section, wxSizerFlags().Expand().Border());
	wxFlexGridSizer* event_list_settings_sizer = new wxFlexGridSizer(3, wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder());
	event_list_settings_section->Add(event_list_settings_sizer, wxSizerFlags().Expand().Border());
	event_list_settings_sizer->AddGrowableCol(1, 1);

	event_list_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "Font"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->event_list_font_picker = new wxFontPickerCtrl(this, wxID_ANY, this->window->canvas->event_list->font, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL);
	event_list_settings_sizer->Add(this->event_list_font_picker, wxSizerFlags().Expand());
	wxButton* default_event_list_font_button = new wxButton(this, wxID_ANY, "Default");
	event_list_settings_sizer->Add(default_event_list_font_button, wxSizerFlags().Expand());
	default_event_list_font_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultEventListFontButtonClick, this);

	wxStaticBoxSizer* piano_roll_settings_section = new wxStaticBoxSizer(wxVERTICAL, this, "Piano roll");
	outer_sizer->Add(piano_roll_settings_section, wxSizerFlags().Expand().Border(wxRIGHT | wxBOTTOM | wxLEFT));
	wxFlexGridSizer* piano_roll_settings_sizer = new wxFlexGridSizer(3, wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder());
	piano_roll_settings_section->Add(piano_roll_settings_sizer, wxSizerFlags().Expand().Border());
	piano_roll_settings_sizer->AddGrowableCol(1, 1);

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "First note"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_first_note_text_box = new wxTextCtrl(this, wxID_ANY, GetNoteNameFromNumber(this->window->canvas->piano_roll->first_note));
	piano_roll_settings_sizer->Add(this->piano_roll_first_note_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_first_note_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_first_note_button, wxSizerFlags().Expand());
	default_piano_roll_first_note_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultPianoRollFirstNoteButtonClick, this);

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "Last note"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_last_note_text_box = new wxTextCtrl(this, wxID_ANY, GetNoteNameFromNumber(this->window->canvas->piano_roll->last_note));
	piano_roll_settings_sizer->Add(this->piano_roll_last_note_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_last_note_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_last_note_button, wxSizerFlags().Expand());
	default_piano_roll_last_note_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultPianoRollLastNoteButtonClick, this);

	piano_roll_settings_sizer->Add(new wxStaticText(this, wxID_ANY, "Key width"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->piano_roll_key_width_text_box = new wxTextCtrl(this, wxID_ANY, wxString::Format("%ld", this->window->canvas->piano_roll->key_width));
	piano_roll_settings_sizer->Add(this->piano_roll_key_width_text_box, wxSizerFlags().Expand());
	wxButton* default_piano_roll_key_width_button = new wxButton(this, wxID_ANY, "Default");
	piano_roll_settings_sizer->Add(default_piano_roll_key_width_button, wxSizerFlags().Expand());
	default_piano_roll_key_width_button->Bind(wxEVT_BUTTON, &SettingsDialog::OnDefaultPianoRollKeyWidthButtonClick, this);

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER).Border());

	outer_sizer->Fit(this);
}

void SettingsDialog::OnDefaultEventListFontButtonClick(wxCommandEvent& event)
{
	this->event_list_font_picker->SetSelectedFont(this->window->application->default_event_list_font);
	event.Skip();
}

void SettingsDialog::OnDefaultPianoRollFirstNoteButtonClick(wxCommandEvent& event)
{
	this->piano_roll_first_note_text_box->SetValue(GetNoteNameFromNumber(this->window->application->default_piano_roll_first_note));
	event.Skip();
}

void SettingsDialog::OnDefaultPianoRollLastNoteButtonClick(wxCommandEvent& event)
{
	this->piano_roll_last_note_text_box->SetValue(GetNoteNameFromNumber(this->window->application->default_piano_roll_last_note));
	event.Skip();
}

void SettingsDialog::OnDefaultPianoRollKeyWidthButtonClick(wxCommandEvent& event)
{
	this->piano_roll_key_width_text_box->SetValue(wxString::Format("%ld", this->window->application->default_piano_roll_key_width));
	event.Skip();
}

wxString NoteEventType::GetName()
{
	return wxString("Note");
}

bool NoteEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isNoteStartEvent(event));
}

wxString NoteEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Note");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFileNoteStartEvent_getChannel(event) + 1);
		}
		case 4:
		{
			return GetNoteNameFromNumber(MidiFileNoteStartEvent_getNote(event));
		}
		case 5:
		{
			return wxString::Format("%d", MidiFileNoteStartEvent_getVelocity(event));
		}
		case 6:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event))));
		}
		case 7:
		{
			return wxString::Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(event)));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString ControlChangeEventType::GetName()
{
	return wxString("Control change");
}

bool ControlChangeEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE;
}

wxString ControlChangeEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Ctrl");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFileControlChangeEvent_getChannel(event) + 1);
		}
		case 4:
		{
			return wxString::Format("%d", MidiFileControlChangeEvent_getNumber(event) + 1);
		}
		case 5:
		{
			return wxString::Format("%d", MidiFileControlChangeEvent_getValue(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString ProgramChangeEventType::GetName()
{
	return wxString("Program change");
}

bool ProgramChangeEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE;
}

wxString ProgramChangeEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Prog");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFileProgramChangeEvent_getChannel(event) + 1);
		}
		case 4:
		{
			return wxString::Format("%d", MidiFileProgramChangeEvent_getNumber(event) + 1);
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString AftertouchEventType::GetName()
{
	return wxString("Aftertouch");
}

bool AftertouchEventType::Matches(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE) || (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE);
}

wxString AftertouchEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Touch");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
			{
				return wxString::Format("%d", MidiFileKeyPressureEvent_getChannel(event) + 1);
			}
			else
			{
				return wxString::Format("%d", MidiFileChannelPressureEvent_getChannel(event) + 1);
			}
		}
		case 4:
		{
			if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
			{
				return GetNoteNameFromNumber(MidiFileKeyPressureEvent_getNote(event));
			}
			else
			{
				return wxString("-");
			}
		}
		case 5:
		{
			if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
			{
				return wxString::Format("%d", MidiFileKeyPressureEvent_getAmount(event));
			}
			else
			{
				return wxString::Format("%d", MidiFileChannelPressureEvent_getAmount(event));
			}
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString PitchBendEventType::GetName()
{
	return wxString("Pitch bend");
}

bool PitchBendEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL;
}

wxString PitchBendEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Bend");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFilePitchWheelEvent_getChannel(event) + 1);
		}
		case 5:
		{
			return wxString::Format("%d", MidiFilePitchWheelEvent_getValue(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString SystemExclusiveEventType::GetName()
{
	return wxString("System exclusive");
}

bool SystemExclusiveEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_SYSEX;
}

wxString SystemExclusiveEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	// TODO: render bytes as ascii plus hex escapes

	switch (column_number)
	{
		case 0:
		{
			return wxString("Sysex");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString TextEventType::GetName()
{
	return wxString("Text");
}

bool TextEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isTextEvent(event));
}

wxString TextEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Text");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFileTextEvent_getText(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString LyricEventType::GetName()
{
	return wxString("Lyric");
}

bool LyricEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isLyricEvent(event));
}

wxString LyricEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Lyric");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFileLyricEvent_getText(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString MarkerEventType::GetName()
{
	return wxString("Marker");
}

bool MarkerEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isMarkerEvent(event));
}

wxString MarkerEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Marker");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFileMarkerEvent_getText(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString PortEventType::GetName()
{
	return wxString("Port");
}

bool PortEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isPortEvent(event));
}

wxString PortEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Port");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFilePortEvent_getName(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString TempoEventType::GetName()
{
	return wxString("Tempo");
}

bool TempoEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isTempoEvent(event));
}

wxString TempoEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Tempo");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString TimeSignatureEventType::GetName()
{
	return wxString("Time signature");
}

bool TimeSignatureEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isTimeSignatureEvent(event));
}

wxString TimeSignatureEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Time");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(event), MidiFileTimeSignatureEvent_getDenominator(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString KeySignatureEventType::GetName()
{
	return wxString("Key signature");
}

bool KeySignatureEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isKeySignatureEvent(event));
}

wxString KeySignatureEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Key");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->canvas->window->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return GetKeyNameFromNumber(MidiFileKeySignatureEvent_getNumber(event), (bool)(MidiFileKeySignatureEvent_isMinor(event)));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}


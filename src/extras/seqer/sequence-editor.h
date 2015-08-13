#ifndef SEQUENCE_EDITOR_INCLUDED
#define SEQUENCE_EDITOR_INCLUDED

#include <memory>
#include <vector>
#include <wx/wx.h>
#include <midifile.h>

class Window;
class Sequence;
class EventList;
class PianoRoll;
class StepSize;
class Row;
class Step;

wxString GetNoteNameFromNumber(int note_number);
int GetNoteNumberFromName(wxString note_name);
int GetNoteOctave(int note_number);
int SetNoteOctave(int note_number, int octave);
int GetNoteChromatic(int note_number);
int SetNoteChromatic(int note_number, int chromatic);
wxString GetKeyNameFromNumber(int key_number, bool is_minor);
int GetChromaticFromDiatonicInKey(int diatonic, int key_number);

class SequenceEditor: public wxScrolledCanvas
{
public:
	Window* window;
	Sequence* sequence;
	EventList* event_list;
	PianoRoll* piano_roll;
	StepSize* step_size;
	std::vector<Row> rows;
	std::vector<Step> steps;
	std::vector<int> filtered_event_types;
	std::vector<int> filtered_tracks;
	std::vector<int> filtered_channels;
	long current_row_number;
	long current_column_number;

	SequenceEditor(Window* window);
	~SequenceEditor();
	bool Load(wxString filename);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetVisibleHeight();
	long GetNumberOfVisibleRows();
	long GetFirstVisibleY();
	long GetLastVisibleY();
	long GetFirstRowNumberFromStepNumber(long step_number);
	long GetLastRowNumberFromStepNumber(long step_number);
	long GetStepNumberFromRowNumber(long row_number);
	long GetStepNumberFromTick(long tick);
	double GetFractionalStepNumberFromTick(long tick);
	MidiFileEvent_t GetLatestTimeSignatureEventForRowNumber(long row_number);
	bool Filter(MidiFileEvent_t event);
	void SetStepSize(StepSize* step_size, bool prepare = true);
	void ZoomIn(bool prepare = true);
	void ZoomOut(bool prepare = true);
	void RowUp();
	void RowDown();
	void PageUp();
	void PageDown();
};

class Sequence
{
public:
	SequenceEditor* sequence_editor;
	MidiFile_t midi_file;

	Sequence(SequenceEditor* sequence_editor);
	~Sequence();
};

class EventList
{
public:
	SequenceEditor *sequence_editor;
	wxFont font;
	wxColour current_cell_border_color;
	long row_height;
	long column_widths[8];

	EventList(SequenceEditor* sequence_editor);
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
	SequenceEditor *sequence_editor;
	long first_note;
	long last_note;
	long key_width;
	wxColour darker_line_color;
	wxColour lighter_line_color;
	wxColour lightest_line_color;
	wxColour white_key_color;
	wxColour black_key_color;
	wxColour shadow_color;

	PianoRoll(SequenceEditor* sequence_editor);
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

#endif

#ifndef SEQUENCE_EDITOR_INCLUDED
#define SEQUENCE_EDITOR_INCLUDED

class SequenceEditor;
class Sequence;
class Row;
class Step;

#include <vector>
#include <wx/wx.h>
#include <midifile.h>
#include "seqer.h"
#include "event-list.h"
#include "piano-roll.h"
#include "music-math.h"

typedef enum
{
	EVENT_TYPE_INVALID = -1,
	EVENT_TYPE_NOTE,
	EVENT_TYPE_CONTROL_CHANGE,
	EVENT_TYPE_PROGRAM_CHANGE,
	EVENT_TYPE_AFTERTOUCH,
	EVENT_TYPE_PITCH_BEND,
	EVENT_TYPE_SYSTEM_EXCLUSIVE,
	EVENT_TYPE_TEXT,
	EVENT_TYPE_LYRIC,
	EVENT_TYPE_MARKER,
	EVENT_TYPE_PORT,
	EVENT_TYPE_TEMPO,
	EVENT_TYPE_TIME_SIGNATURE,
	EVENT_TYPE_KEY_SIGNATURE,
	EVENT_TYPE_HIGHEST
}
EventType_t;

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
	int insertion_track_number;
	int insertion_channel_number;
	int insertion_note_number;
	int insertion_velocity;

	SequenceEditor(Window* window);
	~SequenceEditor();
	void New();
	bool Load(wxString filename);
	void SetStepSize(StepSize* step_size, bool suppress_refresh = false);
	void ZoomIn();
	void ZoomOut();
	void RowUp();
	void RowDown();
	void PageUp();
	void PageDown();
	void GoToFirstRow();
	void GoToLastRow();
	void ColumnLeft();
	void ColumnRight();
	void GoToColumn(int column_number);
	void GoToNextMarker();
	void GoToPreviousMarker();
	void GoToMarker(wxString marker_name);
	void InsertNote(int diatonic);

	void RefreshData(bool suppress_refresh = false);
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
	long GetTickFromRowNumber(long row_number);
	long GetRowNumberForEvent(MidiFileEvent_t event);
	MidiFileEvent_t GetLatestTimeSignatureEventForRowNumber(long row_number);
	bool Filter(MidiFileEvent_t event);
	void SetCurrentRowNumber(long current_row_number);
	wxString GetEventTypeName(EventType_t event_type);
	EventType_t GetEventType(MidiFileEvent_t event);
};

class Sequence
{
public:
	SequenceEditor* sequence_editor;
	MidiFile_t midi_file;

	Sequence(SequenceEditor* sequence_editor);
	~Sequence();
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

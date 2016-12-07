#ifndef SEQUENCE_EDITOR_INCLUDED
#define SEQUENCE_EDITOR_INCLUDED

class SequenceEditor;
class Sequence;
class Row;
class Step;
class RowLocator;
class UndoCommand;

#include <functional>
#include <list>
#include <vector>
#include <wx/wx.h>
#include <wx/cmdproc.h>
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
	long last_row_number;
	long current_column_number;
	int insertion_track_number;
	int insertion_channel_number;
	int insertion_note_number;
	int insertion_velocity;
	int insertion_end_velocity;

	SequenceEditor(Window* window, SequenceEditor* existing_sequence_editor = NULL);
	~SequenceEditor();
	bool IsModified();
	bool IsLastEditorForSequence();
	wxString GetFilename();
	void New();
	bool Load(wxString filename);
	bool Save();
	bool SaveAs(wxString filename);
	void SetStepSize(StepSize* step_size);
	void ZoomIn();
	void ZoomOut();
	void SetFilters(std::vector<int> filtered_event_types, std::vector<int> filtered_tracks, std::vector<int> filtered_channels);
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
	void InsertMarker();
	void DeleteRow();
	void EnterValue();
	void SmallIncrease();
	void SmallDecrease();
	void LargeIncrease();
	void LargeDecrease();
	void Quantize();

	void RefreshData();
	void RefreshDisplay();
	void OnDraw(wxDC& dc);
	void UpdateScrollbar();
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
	long GetRowNumberFromTick(long tick);
	long GetNumberOfTicksPerPixel(long step_number);
	long GetRowNumberForEvent(MidiFileEvent_t event);
	MidiFileEvent_t GetLatestTimeSignatureEventForRowNumber(long row_number);
	bool Filter(MidiFileEvent_t event);
	void SetCurrentRowNumber(long current_row_number);
	RowLocator GetLocatorFromRowNumber(long row_number);
	long GetRowNumberFromLocator(RowLocator row_locator);
	wxString GetEventTypeName(EventType_t event_type);
	EventType_t GetEventType(MidiFileEvent_t event);
};

class Sequence
{
public:
	std::list<SequenceEditor*> sequence_editors;
	wxCommandProcessor* undo_command_processor;
	wxString filename;
	MidiFile_t midi_file;
	bool is_modified;

	Sequence();
	~Sequence();
    void AddSequenceEditor(SequenceEditor* sequence_editor);
    void RemoveSequenceEditor(SequenceEditor* sequence_editor);
    void RefreshData();
    void RefreshDisplay();
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

class RowLocator
{
public:
	MidiFileEvent_t event;
	long tick;
};

class UndoCommand: public wxCommand
{
public:
	std::function<void ()> undo_callback;
	std::function<void ()> cleanup_when_undone_callback;
	std::function<void ()> redo_callback;
	std::function<void ()> cleanup_when_done_callback;
	bool has_been_undone;

	UndoCommand(std::function<void ()> undo_callback, std::function<void ()> cleanup_when_undone_callback, std::function<void ()> redo_callback, std::function<void ()> cleanup_when_done_callback);
	~UndoCommand();
	bool Do();
	bool Undo();
};

#endif

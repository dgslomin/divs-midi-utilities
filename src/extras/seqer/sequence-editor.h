#ifndef SEQUENCE_EDITOR_INCLUDED
#define SEQUENCE_EDITOR_INCLUDED

class SequenceEditor;
class Sequence;
class Step;
class Row;
class Cell;
class EmptyRow;
class EmptyRowTimeCell;
class RowLocator;
class UndoCommand;

#include <functional>
#include <list>
#include <vector>
#include <wx/wx.h>
#include <wx/cmdproc.h>
#include <midifile.h>
#include "event-list.h"
#include "event-types.h"
#include "music-math.h"
#include "piano-roll.h"
#include "seqer.h"

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
	std::vector<EventType*> filtered_event_types;
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
	bool overwrite_mode;

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
	void SetFilters(std::vector<EventType*> filtered_event_types, std::vector<int> filtered_tracks, std::vector<int> filtered_channels);
	void SetOverwriteMode(bool overwrite_mode);
	void SelectCurrent();
	void SelectAll();
	void SelectNone();
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
	bool Filter(EventType* event_type, MidiFileEvent_t event);
	void SetCurrentRowNumber(long current_row_number);
	RowLocator GetLocatorFromRowNumber(long row_number);
	long GetRowNumberFromLocator(RowLocator row_locator);
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

class Step
{
public:
	long first_row_number;
	long last_row_number;

	Step(long row_number);
};

class Row
{
public:
	SequenceEditor* sequence_editor;
	long step_number;
	MidiFileEvent_t event;
	wxString label = wxEmptyString;
	Cell* cells[7];

	Row(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
	virtual void Delete();
};

class Cell
{
public:
	Row* row;
	wxString label = wxEmptyString;

	Cell(Row* row);
	virtual wxString GetValueText();
	virtual void EnterValue();
	virtual void SmallIncrease();
	virtual void SmallDecrease();
	virtual void LargeIncrease();
	virtual void LargeDecrease();
	virtual void Quantize();
};

class EmptyRow: public Row
{
public:
	EmptyRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class EmptyRowTimeCell: public Cell
{
public:
	EmptyRowTimeCell(Row* row);
	wxString GetValueText();
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

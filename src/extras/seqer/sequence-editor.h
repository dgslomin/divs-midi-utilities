#ifndef SEQUENCE_EDITOR_INCLUDED
#define SEQUENCE_EDITOR_INCLUDED

class SequenceEditor;
class EventList;
class PianoRoll;
class Step;
class Row;
class Cell;
class EmptyRow;
class EmptyRowTimeCell;
class EventTypeManager;
class EventType;
class EventTypeCell;
class RowLocator;
class UndoCommand;
class Sequence;

#include <functional>
#include <list>
#include <set>
#include <vector>
#include <wx/wx.h>
#include <wx/cmdproc.h>
#include <midifile.h>
#include "music-math.h"
#include "seqer.h"

#define EVENT_LIST_NUMBER_OF_COLUMNS 8

class SequenceEditor: public wxScrolledCanvas
{
public:
	Window* window;
	Sequence* sequence;
	EventList* event_list;
	PianoRoll* piano_roll;
	StepSize* step_size;
	std::vector<Row*> rows;
	std::vector<Step*> steps;
	std::set<EventType*> filtered_event_types;
	std::set<MidiFileTrack_t> filtered_tracks;
	std::set<int> filtered_channels;
	long current_row_number;
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
	void SetFilters(std::set<EventType*> filtered_event_types, std::set<MidiFileTrack_t> filtered_tracks, std::set<int> filtered_channels);
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

	void ClearData();
	void RefreshData();
	void RefreshDisplay();
	void OnDraw(wxDC& dc);
	void UpdateScrollbar();
	long GetVisibleWidth();
	long GetVisibleHeight();
	long GetNumberOfVisibleRows();
	long GetFirstVisibleY();
	long GetLastVisibleY();
	Row* GetRow(long row_number);
	long GetFirstRowNumberFromStepNumber(long step_number);
	long GetLastRowNumberFromStepNumber(long step_number);
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

class EventList
{
public:
	SequenceEditor *sequence_editor;
	wxFont font;
	wxColour current_cell_border_color;
	wxColour selected_row_background_color;
	wxColour selected_row_text_color;
	long row_height;
	long column_widths[EVENT_LIST_NUMBER_OF_COLUMNS];

	EventList(SequenceEditor* sequence_editor);
	void RefreshData();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetFirstVisibleRowNumber();
	long GetLastVisibleRowNumber();
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
	void RefreshData();
	void OnDraw(wxDC& dc);
	long GetWidth();
	long GetYFromStepNumber(double step_number);
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
	EventType* event_type;
	Cell* cells[EVENT_LIST_NUMBER_OF_COLUMNS];
	bool selected;

	Row(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
	~Row();
	virtual void Delete();
	long GetTick();
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
	EmptyRow(SequenceEditor* sequence_editor, long step_number);
};

class EmptyRowTimeCell: public Cell
{
public:
	EmptyRowTimeCell(Row* row);
	wxString GetValueText();
};

class EventTypeManager
{
public:
	std::vector<EventType*> event_types;

	static EventTypeManager* GetInstance();

private:
	EventTypeManager();

public:
	EventType* GetEventType(MidiFileEvent_t event);
};

class EventType
{
public:
	wxString name;
	wxString short_name;

	virtual ~EventType() = 0;
	virtual bool MatchesEvent(MidiFileEvent_t event) = 0;
	virtual Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event) = 0;
};

class EventTypeCell: public Cell
{
public:
	EventTypeCell(Row* row);
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

#endif

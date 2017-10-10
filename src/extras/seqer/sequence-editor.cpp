
#include <algorithm>
#include <functional>
#include <vector>
#include <wx/wx.h>
#include <wx/cmdproc.h>
#include <wx/filename.h>
#include <midifile.h>
#include "event-list.h"
#include "event-types.h"
#include "music-math.h"
#include "piano-roll.h"
#include "seqer.h"
#include "sequence-editor.h"

#ifdef __WXMSW__
#define CANVAS_BORDER wxBORDER_THEME
#else
#define CANVAS_BORDER wxBORDER_DEFAULT
#endif

SequenceEditor::SequenceEditor(Window* window, SequenceEditor* existing_sequence_editor): wxScrolledCanvas(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | CANVAS_BORDER)
{
	this->window = window;
	this->sequence = (existing_sequence_editor == NULL) ? new Sequence() : existing_sequence_editor->sequence;
	this->sequence->AddSequenceEditor(this);
	this->event_list = new EventList(this);
	this->piano_roll = new PianoRoll(this);
	this->step_size = new StepsPerMeasureSize(this);
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 1;
	this->insertion_track_number = 1;
	this->insertion_channel_number = 0;
	this->insertion_note_number = 60;
	this->insertion_velocity = 64;
	this->insertion_end_velocity = 0;
	this->overwrite_mode = false;
#ifndef __WXOSX__
	this->SetDoubleBuffered(true);
#endif
	this->DisableKeyboardScrolling();
	this->SetBackgroundColour(*wxWHITE);

	this->Bind(wxEVT_CHAR_HOOK, [=](wxKeyEvent& event) {
		this->SetCursor(wxCURSOR_BLANK);
		event.Skip();
	});

	this->Bind(wxEVT_MOTION, [=](wxMouseEvent& event) {
		this->SetCursor(wxCURSOR_ARROW);
		event.Skip();
	});

	this->RefreshData();
}

SequenceEditor::~SequenceEditor()
{
	this->sequence->RemoveSequenceEditor(this);
	delete this->event_list;
	delete this->piano_roll;
	delete this->step_size;
}

bool SequenceEditor::IsModified()
{
	return this->sequence->is_modified;
}

bool SequenceEditor::IsLastEditorForSequence()
{
	return (this->sequence->sequence_editors.size() == 1);
}

wxString SequenceEditor::GetFilename()
{
	return this->sequence->filename;
}

void SequenceEditor::New()
{
	this->sequence->RemoveSequenceEditor(this);
	this->sequence = new Sequence();
	this->sequence->AddSequenceEditor(this);
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 0;
	this->RefreshData();
}

bool SequenceEditor::Load(wxString filename)
{
	MidiFile_t new_midi_file = MidiFile_load((char *)(filename.ToStdString().c_str()));
	if (new_midi_file == NULL) return false;
	this->sequence->RemoveSequenceEditor(this);
	this->sequence = new Sequence();
	this->sequence->AddSequenceEditor(this);
	this->sequence->filename = filename;
	MidiFile_free(this->sequence->midi_file);
	this->sequence->midi_file = new_midi_file;
	this->current_row_number = 0;
	this->last_row_number = 0;
	this->current_column_number = 0;
	this->SetStepSize(new StepsPerMeasureSize(this));
	this->RefreshData();
	return true;
}

bool SequenceEditor::Save()
{
	if (this->sequence->filename == wxEmptyString) return false;

	if (MidiFile_save(this->sequence->midi_file, (char *)(this->sequence->filename.ToStdString().c_str())) == 0)
	{
		this->sequence->is_modified = false;
		this->sequence->RefreshDisplay();
		return true;
	}
	else
	{
		return false;
	}
}

bool SequenceEditor::SaveAs(wxString filename)
{
	if (MidiFile_save(this->sequence->midi_file, (char *)(filename.ToStdString().c_str())) == 0)
	{
		this->sequence->filename = filename;
		this->sequence->is_modified = false;
		this->sequence->RefreshDisplay();
		return true;
	}
	else
	{
		return false;
	}
}

void SequenceEditor::SetStepSize(StepSize* step_size)
{
	RowLocator row_locator = this->GetLocatorFromRowNumber(this->current_row_number);
	delete this->step_size;
	this->step_size = step_size;
	this->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberFromLocator(row_locator));
	this->RefreshDisplay();
}

void SequenceEditor::ZoomIn()
{
	this->SetStepSize(this->step_size->ZoomIn());
}

void SequenceEditor::ZoomOut()
{
	this->SetStepSize(this->step_size->ZoomOut());
}

void SequenceEditor::SetFilters(std::vector<EventType*> filtered_event_types, std::vector<int> filtered_tracks, std::vector<int> filtered_channels)
{
	RowLocator row_locator = this->GetLocatorFromRowNumber(this->current_row_number);
	this->filtered_event_types = filtered_event_types;
	this->filtered_tracks = filtered_tracks;
	this->filtered_channels = filtered_channels;
	this->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberFromLocator(row_locator));
	this->RefreshDisplay();
}

void SequenceEditor::SetOverwriteMode(bool overwrite_mode)
{
	this->overwrite_mode = overwrite_mode;
}

void SequenceEditor::SelectCurrent()
{
}

void SequenceEditor::SelectAll()
{
}

void SequenceEditor::SelectNone()
{
}

void SequenceEditor::RowUp()
{
	this->SetCurrentRowNumber(std::max<long>(this->current_row_number - 1, 0));
}

void SequenceEditor::RowDown()
{
	this->SetCurrentRowNumber(current_row_number + 1);
}

void SequenceEditor::PageUp()
{
	this->SetCurrentRowNumber(std::max<long>(this->current_row_number - this->GetNumberOfVisibleRows(), 0));
}

void SequenceEditor::PageDown()
{
	this->SetCurrentRowNumber(current_row_number + this->GetNumberOfVisibleRows());
}

void SequenceEditor::GoToFirstRow()
{
	this->SetCurrentRowNumber(0);
}

void SequenceEditor::GoToLastRow()
{
	this->SetCurrentRowNumber(std::max<long>(this->rows.size() - 1, this->last_row_number));
}

void SequenceEditor::ColumnLeft()
{
	this->current_column_number = std::max<long>(this->current_column_number - 1, 1);
	this->RefreshDisplay();
}

void SequenceEditor::ColumnRight()
{
	this->current_column_number = std::min<long>(this->current_column_number + 1, 7);
	this->RefreshDisplay();
}

void SequenceEditor::GoToColumn(int column_number)
{
	this->current_column_number = std::min<long>(std::max<long>(column_number, 1), 7);
	this->RefreshDisplay();
}

void SequenceEditor::GoToNextMarker()
{
	for (long row_number = this->current_row_number + 1; row_number < this->rows.size(); row_number++)
	{
		if (MidiFileEvent_isMarkerEvent(this->rows[row_number].event))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long current_tick = this->GetTickFromRowNumber(this->current_row_number);

	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(MidiFile_getFirstTrack(this->sequence->midi_file)); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isMarkerEvent(event))
		{
			long tick = MidiFileEvent_getTick(event);

			if (tick > current_tick)
			{
				this->SetCurrentRowNumber(this->GetRowNumberFromTick(tick));
				return;
			}
		}
	}
}

void SequenceEditor::GoToPreviousMarker()
{
	for (long row_number = std::min<long>(this->current_row_number, this->rows.size()) - 1; row_number >= 0; row_number--)
	{
		if (MidiFileEvent_isMarkerEvent(this->rows[row_number].event))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long current_tick = this->GetTickFromRowNumber(this->current_row_number);

	for (MidiFileEvent_t event = MidiFileTrack_getLastEvent(MidiFile_getFirstTrack(this->sequence->midi_file)); event != NULL; event = MidiFileEvent_getPreviousEventInTrack(event))
	{
		if (MidiFileEvent_isMarkerEvent(event))
		{
 			long tick = MidiFileEvent_getTick(event);

			if (tick < current_tick)
			{
				this->SetCurrentRowNumber(this->GetRowNumberFromTick(tick));
				return;
			}
		}
	}
}

void SequenceEditor::GoToMarker(wxString marker_name)
{
	for (long row_number = 0; row_number < this->rows.size(); row_number++)
	{
		MidiFileEvent_t event = rows[row_number].event;

		if (MidiFileEvent_isMarkerEvent(event) && (marker_name.Cmp(MidiFileMarkerEvent_getText(event)) == 0))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long tick = MidiFile_getTickFromMarker(this->sequence->midi_file, (char *)(marker_name.ToStdString().c_str()));
	if (tick >= 0) this->SetCurrentRowNumber(this->GetRowNumberFromTick(tick));
}

void SequenceEditor::DeleteRow()
{
	if (this->current_row_number >= this->rows.size()) return;
	this->rows[this->current_row_number]->cells[this->current_column_number]->Delete();
}

void SequenceEditor::EnterValue()
{
	if (this->current_row_number >= this->rows.size()) return;
    this->rows[this->current_row_number]->cells[this->current_column_number]->EnterValue();
}

void SequenceEditor::SmallIncrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	this->rows[this->current_row_number]->cells[this->current_column_number]->SmallIncrease();
}

void SequenceEditor::SmallDecrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	this->rows[this->current_row_number]->cells[this->current_column_number]->SmallDecrease();
}

void SequenceEditor::LargeIncrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	this->rows[this->current_row_number]->cells[this->current_column_number]->LargeIncrease();
}

void SequenceEditor::LargeDecrease()
{
	if (this->current_row_number >= this->rows.size()) return;
	this->rows[this->current_row_number]->cells[this->current_column_number]->LargeDecrease();
}

void SequenceEditor::Quantize()
{
	if (this->current_row_number >= this->rows.size()) return;
	this->rows[this->current_row_number]->cells[this->current_column_number]->Quantize();
}

void SequenceEditor::RefreshData()
{
	this->rows.clear();
	this->steps.clear();

	if (this->sequence->midi_file != NULL)
	{
		long last_step_number = -1;

		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
            EventType* event_type = event_types->GetEventType(event);
			if (!this->Filter(event_type, event)) continue;

			long step_number = this->GetStepNumberFromTick(MidiFileEvent_getTick(event));

			while (last_step_number < step_number - 1)
			{
				last_step_number++;
				this->rows.push_back(Row(last_step_number, NULL));
				this->steps.push_back(Step(this->rows.size() - 1));
			}

			this->rows.push_back(event_type->GetRow(this, step_number, event));

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

	this->event_list->RefreshData();
	this->piano_roll->RefreshData();
	this->UpdateScrollbar();
	this->sequence->RefreshDisplay();
}

void SequenceEditor::RefreshDisplay()
{
#ifdef __WXOSX__
	this->window->SetTitle(wxString::Format("%s - Seqer", (this->sequence->filename == wxEmptyString) ? "Untitled" : wxFileName(this->sequence->filename).GetFullName()));
	this->window->OSXSetModified(this->sequence->is_modified);
#else
	this->window->SetTitle(wxString::Format("%s%s - Seqer", this->sequence->is_modified ? "*" : "", (this->sequence->filename == wxEmptyString) ? "Untitled" : wxFileName(this->sequence->filename).GetFullName()));
#endif

	this->window->SetStatusText(this->sequence_editor->rows[row_number]->cells[column_number]->label, 0);
	this->Refresh();
}

void SequenceEditor::OnDraw(wxDC& dc)
{
	this->piano_roll->OnDraw(dc);
	this->event_list->OnDraw(dc);
}

void SequenceEditor::UpdateScrollbar()
{
	this->SetScrollbars(0, this->event_list->row_height, 0, std::max<long>(this->rows.size(), this->last_row_number + 1), 0, this->GetScrollPos(wxVERTICAL));

	if (this->current_row_number < this->event_list->GetFirstVisibleRowNumber())
	{
		this->Scroll(wxDefaultCoord, this->current_row_number);
	}
	else if (this->current_row_number > event_list->GetLastVisibleRowNumber() - 2)
	{
		this->Scroll(wxDefaultCoord, this->current_row_number - this->GetNumberOfVisibleRows() + 1);
	}
}

long SequenceEditor::GetVisibleWidth()
{
	return this->GetClientSize().GetWidth();
}

long SequenceEditor::GetVisibleHeight()
{
	return this->GetClientSize().GetHeight();
}

long SequenceEditor::GetNumberOfVisibleRows()
{
	return this->GetVisibleHeight() / this->event_list->row_height;
}

long SequenceEditor::GetFirstVisibleY()
{
	return this->event_list->GetYFromRowNumber(this->event_list->GetFirstVisibleRowNumber());
}

long SequenceEditor::GetLastVisibleY()
{
	return this->GetFirstVisibleY() + this->GetVisibleHeight();
}

long SequenceEditor::GetFirstRowNumberFromStepNumber(long step_number)
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

long SequenceEditor::GetLastRowNumberFromStepNumber(long step_number)
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

long SequenceEditor::GetStepNumberFromRowNumber(long row_number)
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

long SequenceEditor::GetStepNumberFromTick(long tick)
{
	return (long)(this->GetFractionalStepNumberFromTick(tick));
}

double SequenceEditor::GetFractionalStepNumberFromTick(long tick)
{
	return this->step_size->GetStepFromTick(tick);
}

long SequenceEditor::GetTickFromRowNumber(long row_number)
{
	if (row_number < this->rows.size())
	{
		MidiFileEvent_t event = this->rows[row_number].event;
		if (event != NULL) return MidiFileEvent_getTick(event);
	}

	return this->step_size->GetTickFromStep(this->GetStepNumberFromRowNumber(row_number));
}

long SequenceEditor::GetRowNumberFromTick(long tick)
{
	long step_number = this->GetStepNumberFromTick(tick);
	long first_row_number = this->GetFirstRowNumberFromStepNumber(step_number);
	long last_row_number = this->GetLastRowNumberFromStepNumber(step_number);

	if (first_row_number < this->rows.size())
	{
		for (long row_number = first_row_number; row_number <= last_row_number; row_number++)
		{
			if (MidiFileEvent_getTick(this->rows[row_number].event) >= tick) return row_number;
		}
	}

	return first_row_number;
}

long SequenceEditor::GetNumberOfTicksPerPixel(long step_number)
{
	long step_tick = this->step_size->GetTickFromStep(step_number);
	long next_step_tick = this->step_size->GetTickFromStep(step_number + 1);
	long number_of_rows = this->GetLastRowNumberFromStepNumber(step_number) - this->GetFirstRowNumberFromStepNumber(step_number) + 1;
	return std::max<long>((next_step_tick - step_tick) / (number_of_rows * this->event_list->row_height), 1);
}

long SequenceEditor::GetRowNumberForEvent(MidiFileEvent_t event)
{
	for (long i = 0; i < this->rows.size(); i++)
	{
		if (rows[i].event == event)
		{
			return i;
		}
	}

	return -1;
}

MidiFileEvent_t SequenceEditor::GetLatestTimeSignatureEventForRowNumber(long row_number)
{
	return MidiFile_getLatestTimeSignatureEventForTick(this->sequence->midi_file, this->step_size->GetTickFromStep(this->GetStepNumberFromRowNumber(row_number)));
}

bool SequenceEditor::Filter(EventType* event_type, MidiFileEvent_t event)
{
	if (event_type == NULL) return false;

	if (this->filtered_event_types.size() > 0)
	{
		bool matched = false;

		for (int i = 0; i < this->filtered_event_types.size(); i++)
		{
			if (event_type == this->filtered_event_types[i])
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

void SequenceEditor::SetCurrentRowNumber(long current_row_number)
{
	if (current_row_number >= 0)
	{
		this->current_row_number = current_row_number;
		if (current_row_number > this->last_row_number) this->last_row_number = current_row_number;
	}

	this->UpdateScrollbar();
	this->RefreshDisplay();
}

RowLocator SequenceEditor::GetLocatorFromRowNumber(long row_number)
{
	RowLocator row_locator = RowLocator();
	row_locator.event = (row_number < this->rows.size()) ? this->rows[row_number].event : NULL;
	row_locator.tick = this->GetTickFromRowNumber(row_number);
	return row_locator;
}

long SequenceEditor::GetRowNumberFromLocator(RowLocator row_locator)
{
	if (row_locator.event == NULL)
	{
		return this->GetFirstRowNumberFromStepNumber(this->GetStepNumberFromTick(row_locator.tick));
	}
	else
	{
		long row_number = this->GetRowNumberForEvent(row_locator.event);

		if (row_number < 0)
		{
			return this->GetFirstRowNumberFromStepNumber(this->GetStepNumberFromTick(MidiFileEvent_getTick(row_locator.event)));
		}
		else
		{
			return row_number;
		}
	}
}

Sequence::Sequence()
{
	this->undo_command_processor = new wxCommandProcessor();
	this->filename = wxEmptyString;
	this->midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	this->is_modified = false;
}

Sequence::~Sequence()
{
	MidiFile_free(this->midi_file);
	delete this->undo_command_processor;
}

void Sequence::AddSequenceEditor(SequenceEditor* sequence_editor)
{
	this->sequence_editors.push_back(sequence_editor);
}

void Sequence::RemoveSequenceEditor(SequenceEditor* sequence_editor)
{
	this->sequence_editors.remove(sequence_editor);
	if (this->sequence_editors.empty()) delete this;
}

void Sequence::RefreshData()
{
	this->is_modified = true;
	for (std::list<SequenceEditor*>::iterator sequence_editor_iterator = this->sequence_editors.begin(); sequence_editor_iterator != this->sequence_editors.end(); sequence_editor_iterator++) (*sequence_editor_iterator)->RefreshData();
}

void Sequence::RefreshDisplay()
{
	for (std::list<SequenceEditor*>::iterator sequence_editor_iterator = this->sequence_editors.begin(); sequence_editor_iterator != this->sequence_editors.end(); sequence_editor_iterator++) (*sequence_editor_iterator)->RefreshDisplay();
}

Step::Step(long row_number)
{
	this->first_row_number = row_number;
	this->last_row_number = row_number;
}

Row::Row(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	this->sequence_editor = sequence_editor;
	this->step_number = step_number;
	this->event = event;
}

void Row::Delete()
{
}

Cell(Row* row)
{
	this->row = row;
}

wxString Cell::GetValueText()
{
	return wxEmptyString;
}

void Cell::EnterValue()
{
}

void Cell::SmallIncrease()
{
}

void Cell::SmallDecrease()
{
}

void Cell::LargeIncrease()
{
    this->SmallIncrease();
}

void Cell::LargeDecrease()
{
    this->SmallDecrease();
}

void Cell::Quantize()
{
}

EmptyRow::EmptyRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
}

EmptyRowTimeCell::EmptyRowTimeCell(Row* row): Cell(row)
{
}

wxString EmptyRowTimeCell::GetValueText()
{
    SequenceEditor* sequence_editor = this->row->sequence_editor;
	long step_number = this->row->step_number;
	return sequence_editor->step_size->GetTimeStringFromTick(sequence_editor->step_size->GetTickFromStep(step_number));
}

UndoCommand::UndoCommand(std::function<void ()> undo_callback, std::function<void ()> cleanup_when_undone_callback, std::function<void ()> redo_callback, std::function<void ()> cleanup_when_done_callback): wxCommand(true)
{
	this->undo_callback = undo_callback;
	this->cleanup_when_undone_callback = cleanup_when_undone_callback;
	this->redo_callback = redo_callback;
	this->cleanup_when_done_callback = cleanup_when_done_callback;
	this->has_been_undone = false;
}

UndoCommand::~UndoCommand()
{
	if (this->has_been_undone)
	{
		if (this->cleanup_when_undone_callback != NULL) this->cleanup_when_undone_callback();
	}
	else
	{
		if (this->cleanup_when_done_callback != NULL) this->cleanup_when_done_callback();
	}
}

bool UndoCommand::Do()
{
	if (this->has_been_undone)
	{
		if (this->redo_callback != NULL) this->redo_callback();
		this->has_been_undone = false;
	}

	return true;
}

bool UndoCommand::Undo()
{
	if (this->undo_callback != NULL) this->undo_callback();
	this->has_been_undone = true;
	return true;
}


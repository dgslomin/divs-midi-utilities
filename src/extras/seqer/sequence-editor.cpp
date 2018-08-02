
#include <algorithm>
#include <functional>
#include <set>
#include <wx/wx.h>
#include <wx/cmdproc.h>
#include <wx/filename.h>
#include <midifile.h>
#include <midifile-extensions.h>
#include "aftertouch-event.h"
#include "control-change-event.h"
#include "color.h"
#include "key-signature-event.h"
#include "lyric-event.h"
#include "marker-event.h"
#include "music-math.h"
#include "note-event.h"
#include "null-event.h"
#include "pitch-bend-event.h"
#include "port-event.h"
#include "program-change-event.h"
#include "seqer.h"
#include "sequence-editor.h"
#include "system-exclusive-event.h"
#include "tempo-event.h"
#include "text-event.h"
#include "time-signature-event.h"

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
	this->ClearData();
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
	this->current_column_number = 1;
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
	this->current_column_number = 1;
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
	delete this->step_size;
	this->step_size = step_size;
	this->RefreshData();
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

void SequenceEditor::SetFilters(std::set<EventType*> filtered_event_types, std::set<MidiFileTrack_t> filtered_tracks, std::set<int> filtered_channels)
{
	this->filtered_event_types = filtered_event_types;
	this->filtered_tracks = filtered_tracks;
	this->filtered_channels = filtered_channels;
	this->RefreshData();
	this->RefreshDisplay();
}

void SequenceEditor::SetOverwriteMode(bool overwrite_mode)
{
	this->overwrite_mode = overwrite_mode;
}

void SequenceEditor::SelectCurrent()
{
	Row* row = this->GetRow(this->current_row_number);
	this->SelectRow(row, !this->RowIsSelected(row));
	this->RefreshData();
}

void SequenceEditor::SelectAll()
{
	MidiFile_clearSelection(this->sequence->midi_file);
	for (long row_number = 0; row_number < this->rows.size(); row_number++) this->SelectRow(this->GetRow(row_number), true);
	this->RefreshData();
}

void SequenceEditor::SelectNone()
{
	MidiFile_clearSelection(this->sequence->midi_file);
	this->RefreshData();
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
	this->SetCurrentRowNumber(this->rows.size() - 1);
}

void SequenceEditor::ScrollUp()
{
	long first_visible_row_number = this->event_list->GetFirstVisibleRowNumber();

	if (first_visible_row_number == 0)
	{
		this->RowUp();
	}
	else
	{
		long last_visible_row_number = this->event_list->GetLastVisibleRowNumber();

		first_visible_row_number--;
		last_visible_row_number--;

		if (this->current_row_number > last_visible_row_number - 2)
		{
			this->SetCurrentRowNumber(this->current_row_number - 1);
		}

		this->Scroll(wxDefaultCoord, first_visible_row_number);
		this->UpdateScrollbar();
		this->RefreshDisplay();
	}
}

void SequenceEditor::ScrollDown()
{
	long first_visible_row_number = this->event_list->GetFirstVisibleRowNumber();
	long last_visible_row_number = this->event_list->GetLastVisibleRowNumber();

	first_visible_row_number++;
	last_visible_row_number++;

	if (this->current_row_number < first_visible_row_number)
	{
		this->SetCurrentRowNumber(this->current_row_number + 1);
	}

	this->Scroll(wxDefaultCoord, first_visible_row_number);
	this->UpdateScrollbar();
	this->RefreshDisplay();
}

void SequenceEditor::ColumnLeft()
{
	this->current_column_number = std::max<long>(this->current_column_number - 1, 1);
	this->RefreshDisplay();
}

void SequenceEditor::ColumnRight()
{
	this->current_column_number = std::min<long>(this->current_column_number + 1, EVENT_LIST_NUMBER_OF_COLUMNS - 1);
	this->RefreshDisplay();
}

void SequenceEditor::GoToColumn(int column_number)
{
	this->current_column_number = std::min<long>(std::max<long>(column_number, 1), EVENT_LIST_NUMBER_OF_COLUMNS - 1);
	this->RefreshDisplay();
}

void SequenceEditor::GoToNextMarker()
{
	for (long row_number = this->current_row_number + 1; row_number < this->rows.size(); row_number++)
	{
		if (MidiFileEvent_isMarkerEvent(this->GetRow(row_number)->event))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long current_tick = this->GetTickFromRow(this->GetRow(this->current_row_number));

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
	for (long row_number = this->current_row_number - 1; row_number >= 0; row_number--)
	{
		if (MidiFileEvent_isMarkerEvent(this->GetRow(row_number)->event))
		{
			this->SetCurrentRowNumber(row_number);
			return;
		}
	}

	long current_tick = this->GetTickFromRow(this->GetRow(this->current_row_number));

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
		MidiFileEvent_t event = this->GetRow(row_number)->event;

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
	if (MidiFile_hasSelection(this->sequence->midi_file))
	{
		for (long row_number = 0; row_number < this->rows.size(); row_number++)
		{
			Row* row = this->GetRow(row_number);
			if (this->RowIsSelected(row)) row->event_type->DeleteRow(this, row);
		}
	}
	else
	{
		Row* row = this->GetRow(this->current_row_number);
		row->event_type->DeleteRow(this, row);
	}

	this->sequence->RefreshData();
}

void SequenceEditor::EnterValue()
{
	Row* row = this->GetRow(this->current_row_number);
	row->event_type->cells[this->current_column_number]->EnterValue(this, row);
	this->sequence->RefreshData();
}

void SequenceEditor::SmallIncrease()
{
	if (MidiFile_hasSelection(this->sequence->midi_file))
	{
		for (long row_number = 0; row_number < this->rows.size(); row_number++)
		{
			Row* row = this->GetRow(row_number);
			if (this->RowIsSelected(row)) row->event_type->cells[this->current_column_number]->SmallIncrease(this, row);
		}
	}
	else
	{
		Row* row = this->GetRow(this->current_row_number);
		row->event_type->cells[this->current_column_number]->SmallIncrease(this, row);
	}

	this->sequence->RefreshData();
}

void SequenceEditor::SmallDecrease()
{
	if (MidiFile_hasSelection(this->sequence->midi_file))
	{
		for (long row_number = 0; row_number < this->rows.size(); row_number++)
		{
			Row* row = this->GetRow(row_number);
			if (this->RowIsSelected(row)) row->event_type->cells[this->current_column_number]->SmallDecrease(this, row);
		}
	}
	else
	{
		Row* row = this->GetRow(this->current_row_number);
		row->event_type->cells[this->current_column_number]->SmallDecrease(this, row);
	}

	this->sequence->RefreshData();
}

void SequenceEditor::LargeIncrease()
{
	if (MidiFile_hasSelection(this->sequence->midi_file))
	{
		for (long row_number = 0; row_number < this->rows.size(); row_number++)
		{
			Row* row = this->GetRow(row_number);
			if (this->RowIsSelected(row)) row->event_type->cells[this->current_column_number]->LargeIncrease(this, row);
		}
	}
	else
	{
		Row* row = this->GetRow(this->current_row_number);
		row->event_type->cells[this->current_column_number]->LargeIncrease(this, row);
	}

	this->sequence->RefreshData();
}

void SequenceEditor::LargeDecrease()
{
	if (MidiFile_hasSelection(this->sequence->midi_file))
	{
		for (long row_number = 0; row_number < this->rows.size(); row_number++)
		{
			Row* row = this->GetRow(row_number);
			if (this->RowIsSelected(row)) row->event_type->cells[this->current_column_number]->LargeDecrease(this, row);
		}
	}
	else
	{
		Row* row = this->GetRow(this->current_row_number);
		row->event_type->cells[this->current_column_number]->LargeDecrease(this, row);
	}

	this->sequence->RefreshData();
}

void SequenceEditor::Quantize()
{
	if (MidiFile_hasSelection(this->sequence->midi_file))
	{
		for (long row_number = 0; row_number < this->rows.size(); row_number++)
		{
			Row* row = this->GetRow(row_number);
			if (this->RowIsSelected(row)) row->event_type->cells[this->current_column_number]->Quantize(this, row);
		}
	}
	else
	{
		Row* row = this->GetRow(this->current_row_number);
		row->event_type->cells[this->current_column_number]->Quantize(this, row);
	}

	this->sequence->RefreshData();
}

void SequenceEditor::ClearData()
{
	for (int row_number = 0; row_number < this->rows.size(); row_number++) delete this->rows[row_number];
	this->rows.clear();

	for (int step_number = 0; step_number < this->steps.size(); step_number++) delete this->steps[step_number];
	this->steps.clear();
}

void SequenceEditor::RefreshData()
{
	this->ClearData();
	long last_step_number = -1;

	if (this->sequence->midi_file != NULL)
	{
		for (MidiFileEvent_t event = MidiFile_getFirstEvent(this->sequence->midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			EventType* event_type = EventTypeManager::GetInstance()->GetEventType(event);
			if (!this->Filter(event_type, event)) continue;

			long step_number = this->GetStepNumberFromTick(MidiFileEvent_getTick(event));

			while (last_step_number < step_number - 1)
			{
				last_step_number++;
				this->rows.push_back(new Row(last_step_number, NULL, NullEventType::GetInstance()));
				this->steps.push_back(new Step(this->rows.size() - 1));
			}

			this->rows.push_back(new Row(step_number, event, event_type));

			if (step_number == last_step_number)
			{
				this->steps[this->steps.size() - 1]->last_row_number++;
			}
			else
			{
				this->steps.push_back(new Step(this->rows.size() - 1));
			}

			last_step_number = step_number;
		}

		MidiFileEvent_t caret_target = MidiFile_getCaretTarget(this->sequence->midi_file);

		if (caret_target != NULL)
		{
			int caret_target_row_number = this->GetRowNumberForEvent(caret_target);
			if (caret_target_row_number >= 0) this->current_row_number = caret_target_row_number;
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

	this->window->SetStatusText(this->GetRow(this->current_row_number)->event_type->cells[this->current_column_number]->label, 0);
	this->Refresh();
}

void SequenceEditor::OnDraw(wxDC& dc)
{
	this->piano_roll->OnDraw(dc);
	this->event_list->OnDraw(dc);
}

void SequenceEditor::UpdateScrollbar()
{
	this->SetScrollbars(0, this->event_list->row_height, 0, this->rows.size(), 0, this->GetScrollPos(wxVERTICAL));

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

Row* SequenceEditor::GetRow(long row_number)
{
	while (this->rows.size() <= row_number)
	{
		this->rows.push_back(new Row(this->steps.size(), NULL, NullEventType::GetInstance()));
		this->steps.push_back(new Step(this->rows.size() - 1));
	}

	return this->rows[row_number];
}

long SequenceEditor::GetFirstRowNumberFromStepNumber(long step_number)
{
	if (this->steps.size() == 0)
	{
		return step_number;
	}
	else if (step_number < this->steps.size())
	{
		return this->steps[step_number]->first_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1]->last_row_number + step_number - this->steps.size() + 1;
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
		return this->steps[step_number]->last_row_number;
	}
	else
	{
		return this->steps[this->steps.size() - 1]->last_row_number + step_number - this->steps.size() + 1;
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

long SequenceEditor::GetTickFromRow(Row* row)
{
	if (row->event == NULL)
	{
		return this->step_size->GetTickFromStep(row->step_number);
	}
	else
	{
		return MidiFileEvent_getTick(row->event);
	}
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
			if (MidiFileEvent_getTick(this->GetRow(row_number)->event) >= tick) return row_number;
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
		if (this->GetRow(i)->event == event) return i;
	}

	return -1;
}

MidiFileEvent_t SequenceEditor::GetLatestTimeSignatureEventForRowNumber(long row_number)
{
	return MidiFile_getLatestTimeSignatureEventForTick(this->sequence->midi_file, this->step_size->GetTickFromStep(this->GetRow(row_number)->step_number));
}

bool SequenceEditor::Filter(EventType* event_type, MidiFileEvent_t event)
{
	if (event_type == NULL) return false;
	if (!this->filtered_event_types.empty() && this->filtered_event_types.find(event_type) == this->filtered_event_types.end()) return false;
	if (!this->filtered_tracks.empty() && this->filtered_tracks.find(MidiFileEvent_getTrack(event)) == this->filtered_tracks.end()) return false;
	if (!this->filtered_channels.empty() && MidiFileEvent_isVoiceEvent(event) && this->filtered_channels.find(MidiFileVoiceEvent_getChannel(event)) == this->filtered_channels.end()) return false;
	return true;
}

void SequenceEditor::SetCurrentRowNumber(long current_row_number)
{
	if (current_row_number < 0) return;
	Row* row = this->GetRow(current_row_number);
	if (row->event == NULL) row->event = MidiFileTrack_getCreateEmptyStepEvent(MidiFile_getTrackByNumber(this->sequence->midi_file, 0, 1), this->GetTickFromRow(row));
	MidiFileEvent_setCaretTarget(row->event);
	this->current_row_number = current_row_number;
	this->UpdateScrollbar();
	this->RefreshDisplay();
}

bool SequenceEditor::RowIsSelected(Row* row)
{
	return MidiFileEvent_isSelected(row->event);
}

void SequenceEditor::SelectRow(Row* row, bool selected)
{
	if (row->event_type == NullEventType::GetInstance())
	{
		if (selected)
		{
			if (row->event == NULL)
			{
				row->event = MidiFileTrack_createEmptyStepEvent(MidiFile_getTrackByNumber(this->sequence->midi_file, 0, 1), this->GetTickFromRow(row));
				MidiFileEvent_setSelected(row->event, 1);
			}
		}
		else
		{
			if (row->event != NULL)
			{
				MidiFileEvent_setSelected(row->event, 0);
				MidiFileEvent_delete(row->event);
				row->event = NULL;
			}
		}
	}
	else
	{
		MidiFileEvent_setSelected(row->event, selected);
	}
}

EventList::EventList(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->font = this->sequence_editor->window->application->default_event_list_font;
}

void EventList::RefreshData()
{
	wxClientDC dc(this->sequence_editor);
	dc.SetFont(this->font);
	this->row_height = dc.GetCharHeight();

	this->current_cell_border_color = ColorShade(*wxBLACK, 15);
	this->selected_row_background_color = ColorShade(*wxBLACK, 90);

	this->column_widths[0] = dc.GetTextExtent("Marker#").GetWidth();
	this->column_widths[1] = dc.GetTextExtent(this->sequence_editor->step_size->GetTimeStringFromTick(0) + "###").GetWidth();
	this->column_widths[2] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[3] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[4] = dc.GetTextExtent("C#0#").GetWidth();
	this->column_widths[5] = dc.GetTextExtent("000#").GetWidth();
	this->column_widths[6] = dc.GetTextExtent(this->sequence_editor->step_size->GetTimeStringFromTick(0) + "###").GetWidth();
	this->column_widths[7] = dc.GetTextExtent("000#").GetWidth();
}

void EventList::OnDraw(wxDC& dc)
{
	long first_y = this->sequence_editor->GetFirstVisibleY();
	long last_y = this->sequence_editor->GetLastVisibleY();
	long first_step_number = this->sequence_editor->GetRow(this->GetRowNumberFromY(first_y))->step_number;
	long last_step_number = this->sequence_editor->GetRow(this->GetRowNumberFromY(last_y))->step_number;
	long first_row_number = this->GetFirstVisibleRowNumber();
	long last_row_number = this->GetLastVisibleRowNumber();
	long piano_roll_width = this->sequence_editor->piano_roll->GetWidth();
	long width = this->GetVisibleWidth();

	dc.SetPen(wxPen(this->selected_row_background_color));
	dc.SetBrush(wxBrush(this->selected_row_background_color));

	for (long row_number = first_row_number; row_number <= last_row_number; row_number++)
	{
		if (this->sequence_editor->RowIsSelected(this->sequence_editor->GetRow(row_number))) dc.DrawRectangle(piano_roll_width, this->GetYFromRowNumber(row_number), width, this->row_height);
	}

	dc.SetPen(wxPen(this->sequence_editor->piano_roll->lighter_line_color));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->sequence_editor->piano_roll->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(piano_roll_width, y, piano_roll_width + width, y);
	}

	dc.SetPen(this->current_cell_border_color);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(this->GetXFromColumnNumber(this->sequence_editor->current_column_number) - 1, this->GetYFromRowNumber(this->sequence_editor->current_row_number), this->GetColumnWidth(this->sequence_editor->current_column_number) + 1, this->row_height + 1);

	dc.SetFont(this->font);
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetTextForeground(*wxBLACK);

	for (long row_number = first_row_number; row_number <= last_row_number; row_number++)
	{
		Row* row = this->sequence_editor->GetRow(row_number);

		for (long column_number = 0; column_number < EVENT_LIST_NUMBER_OF_COLUMNS; column_number++)
		{
			wxString cell_text = row->event_type->cells[column_number]->GetValueText(this->sequence_editor, row);
			if (!cell_text.IsEmpty()) dc.DrawText(cell_text, this->GetXFromColumnNumber(column_number) + (column_number == 0 ? 1 : 0), this->GetYFromRowNumber(row_number) + 1);
		}
	}
}

long EventList::GetVisibleWidth()
{
	return this->sequence_editor->GetVisibleWidth() - this->sequence_editor->piano_roll->GetWidth();
}

long EventList::GetFirstVisibleRowNumber()
{
	return this->sequence_editor->GetViewStart().y;
}

long EventList::GetLastVisibleRowNumber()
{
	return this->GetFirstVisibleRowNumber() + this->GetRowNumberFromY(this->sequence_editor->GetVisibleHeight()) + 1;
}

long EventList::GetColumnWidth(long column_number)
{
	return this->column_widths[column_number];
}

long EventList::GetXFromColumnNumber(long column_number)
{
	long x = this->sequence_editor->piano_roll->GetWidth();
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

PianoRoll::PianoRoll(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->first_note = this->sequence_editor->window->application->default_piano_roll_first_note;
	this->last_note = this->sequence_editor->window->application->default_piano_roll_last_note;
	this->key_width = this->sequence_editor->window->application->default_piano_roll_key_width;
}

void PianoRoll::RefreshData()
{
	this->darker_line_color = ColorShade(*wxBLACK, 80);
	this->lighter_line_color = ColorShade(*wxBLACK, 85);
	this->white_key_color = *wxWHITE;
	this->black_key_color = ColorShade(*wxBLACK, 90);
	this->event_line_color = ColorShade(*wxBLACK, 50);
	this->event_fill_color = ColorShade(*wxBLACK, 60);
	this->selected_event_line_color = ColorShade(*wxBLACK, 25);
	this->selected_event_fill_color = ColorShade(*wxBLACK, 35);
	this->current_event_line_color = ColorShade(*wxBLACK, 15);
}

void PianoRoll::OnDraw(wxDC& dc)
{
	wxPen pens[] = {wxPen(this->darker_line_color), wxPen(this->lighter_line_color)};
	wxBrush brushes[] = {wxBrush(this->white_key_color), wxBrush(this->black_key_color)};
	long key_pens[] = {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1};
	long key_brushes[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
	long first_y = this->sequence_editor->GetFirstVisibleY();
	long last_y = this->sequence_editor->GetLastVisibleY();
	long width = this->GetWidth();
	long height = this->sequence_editor->GetVisibleHeight();

	for (long note = this->first_note; note <= this->last_note; note++)
	{
		dc.SetPen(pens[key_pens[note % 12]]);
		dc.SetBrush(brushes[key_brushes[note % 12]]);
		dc.DrawRectangle((note - this->first_note) * this->key_width - 1, first_y - 1, this->key_width + 1, height + 2);
	}

	long first_step_number = this->sequence_editor->GetRow(this->sequence_editor->event_list->GetRowNumberFromY(first_y))->step_number;
	long last_step_number = this->sequence_editor->GetRow(this->sequence_editor->event_list->GetRowNumberFromY(last_y))->step_number;
	dc.SetPen(wxPen(this->lighter_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(0, y, width, y);
	}

	wxPen event_pen = wxPen(this->event_line_color);
	wxBrush event_brush = wxBrush(this->event_fill_color);
	wxPen selected_event_pen = wxPen(this->selected_event_line_color);
	wxBrush selected_event_brush = wxBrush(this->selected_event_fill_color);
	wxPen current_event_pen = wxPen(this->current_event_line_color, 2);

	long current_event_x = -1;
	long current_event_y;
	long current_event_width;
	long current_event_height;

	for (int row_number = 0; row_number < this->sequence_editor->rows.size(); row_number++)
	{
		Row* row = this->sequence_editor->GetRow(row_number);

		if (row->event_type == NoteEventType::GetInstance())
		{
			int note = MidiFileNoteOnEvent_getNote(row->event);

			if ((note >= this->first_note) && (note <= this->last_note))
			{
				double event_step_number = this->sequence_editor->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(row->event));
				long event_y = this->GetYFromStepNumber(event_step_number);

				MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(row->event);
				double end_event_step_number = this->sequence_editor->GetFractionalStepNumberFromTick(MidiFileEvent_getTick(end_event));
				long end_event_y = this->GetYFromStepNumber(end_event_step_number);

				if ((event_y <= last_y) && (end_event_y >= first_y))
				{
					long event_x = (note - this->first_note) * this->key_width - 1;
					long event_width = this->key_width + 1;
					long event_height = end_event_y - event_y;

					if (this->sequence_editor->RowIsSelected(row))
					{
						dc.SetPen(selected_event_pen);
						dc.SetBrush(selected_event_brush);
					}
					else
					{
						dc.SetPen(event_pen);
						dc.SetBrush(event_brush);
					}

					dc.DrawRectangle(event_x, event_y, event_width, event_height);

					if (row_number == this->sequence_editor->current_row_number)
					{
						current_event_x = event_x;
						current_event_y = event_y;
						current_event_width = event_width;
						current_event_height = event_height;
					}
				}
			}
		}
	}

	if (current_event_x >= 0)
	{
		dc.SetPen(current_event_pen);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(current_event_x, current_event_y, current_event_width + 1, current_event_height);
	}
}

long PianoRoll::GetWidth()
{
	return this->key_width * (this->last_note - this->first_note + 1);
}

long PianoRoll::GetYFromStepNumber(double step_number)
{
	long step_first_y = this->sequence_editor->event_list->GetYFromRowNumber(this->sequence_editor->GetFirstRowNumberFromStepNumber((long)(step_number)));
	long step_last_y = this->sequence_editor->event_list->GetYFromRowNumber(this->sequence_editor->GetLastRowNumberFromStepNumber((long)(step_number)) + 1);
	return step_first_y + (long)((step_last_y - step_first_y) * (step_number - (long)(step_number)));
}

Step::Step(long row_number)
{
	this->first_row_number = row_number;
	this->last_row_number = row_number;
}

Row::Row(long step_number, MidiFileEvent_t event, EventType* event_type)
{
	this->step_number = step_number;
	this->event = event;
	this->event_type = event_type;
}

EventTypeManager* EventTypeManager::GetInstance()
{
	static EventTypeManager* instance = new EventTypeManager();
	return instance;
}

EventTypeManager::EventTypeManager()
{
	this->event_types.push_back(NoteEventType::GetInstance());
	this->event_types.push_back(ControlChangeEventType::GetInstance());
	this->event_types.push_back(ProgramChangeEventType::GetInstance());
	this->event_types.push_back(AftertouchEventType::GetInstance());
	this->event_types.push_back(PitchBendEventType::GetInstance());
	this->event_types.push_back(SystemExclusiveEventType::GetInstance());
	this->event_types.push_back(TextEventType::GetInstance());
	this->event_types.push_back(LyricEventType::GetInstance());
	this->event_types.push_back(MarkerEventType::GetInstance());
	this->event_types.push_back(PortEventType::GetInstance());
	this->event_types.push_back(TempoEventType::GetInstance());
	this->event_types.push_back(TimeSignatureEventType::GetInstance());
	this->event_types.push_back(KeySignatureEventType::GetInstance());
}

EventType* EventTypeManager::GetEventType(MidiFileEvent_t event)
{
	for (int event_type_number = 0; event_type_number < this->event_types.size(); event_type_number++)
	{
		EventType* event_type = this->event_types[event_type_number];
		if (event_type->MatchesEvent(event)) return event_type;
	}

	return NULL;
}

bool EventType::MatchesEvent(MidiFileEvent_t event)
{
	return false;
}

void EventType::DeleteRow(SequenceEditor* sequence_editor, Row* row)
{
	Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_delete(row->event);
	sequence->RefreshData();
}

wxString Cell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxEmptyString;
}

void Cell::EnterValue(SequenceEditor* sequence_editor, Row* row)
{
}

void Cell::SmallIncrease(SequenceEditor* sequence_editor, Row* row)
{
}

void Cell::SmallDecrease(SequenceEditor* sequence_editor, Row* row)
{
}

void Cell::LargeIncrease(SequenceEditor* sequence_editor, Row* row)
{
	this->SmallIncrease(sequence_editor, row);
}

void Cell::LargeDecrease(SequenceEditor* sequence_editor, Row* row)
{
	this->SmallDecrease(sequence_editor, row);
}

void Cell::Quantize(SequenceEditor* sequence_editor, Row* row)
{
}

wxString EventTypeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return row->event_type->short_name;
}

UndoSnapshot::UndoSnapshot(Sequence* sequence): wxCommand(true)
{
	this->sequence = sequence;
	this->midi_file_buffer = NULL;
}

UndoSnapshot::~UndoSnapshot()
{
	if (this->midi_file_buffer != NULL) free(this->midi_file_buffer);
}

void UndoSnapshot::Swap()
{
	unsigned char* midi_file_buffer_to_activate = this->midi_file_buffer;

	this->midi_file_buffer = (unsigned char*)(malloc(MidiFile_getFileSize(this->sequence->midi_file)));
	MidiFile_saveToBuffer(this->sequence->midi_file, this->midi_file_buffer);

	if (midi_file_buffer_to_activate != NULL)
	{
		MidiFile_free(this->sequence->midi_file);
		this->sequence->midi_file = MidiFile_loadFromBuffer(midi_file_buffer_to_activate);
		this->sequence->RefreshData();
		free(midi_file_buffer_to_activate);
	}
}

bool UndoSnapshot::Do()
{
	this->Swap();
	return true;
}

bool UndoSnapshot::Undo()
{
	this->Swap();
	return true;
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


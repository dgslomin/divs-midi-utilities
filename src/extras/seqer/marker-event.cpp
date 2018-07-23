
#include <wx/wx.h>
#include <midifile.h>
#include "marker-event.h"
#include "sequence-editor.h"

void SequenceEditor::InsertMarker()
{
	this->sequence->undo_command_processor->Submit(new UndoSnapshot(this->sequence));
	MidiFileTrack_t track = MidiFile_getTrackByNumber(this->sequence->midi_file, 0, 1);
	MidiFileEvent_t event = MidiFileTrack_createMarkerEvent(track, this->step_size->GetTickFromStep(this->GetRow(this->current_row_number)->step_number), (char *)(""));
	this->sequence->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));
}

MarkerEventType* MarkerEventType::GetInstance()
{
	static MarkerEventType* instance = new MarkerEventType();
	return instance;
}

MarkerEventType::MarkerEventType()
{
	this->name = wxString("Marker");
	this->short_name = wxString("Marker");
}

bool MarkerEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isMarkerEvent(event);
}

Row* MarkerEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new MarkerEventRow(sequence_editor, step_number, event);
}

MarkerEventRow::MarkerEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->event_type = MarkerEventType::GetInstance();
	this->cells[0] = new EventTypeCell(this);
	this->cells[1] = new MarkerEventTimeCell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new Cell(this);
	this->cells[4] = new MarkerEventNameCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
	this->cells[7] = new Cell(this);
}

void MarkerEventRow::Delete()
{
    Sequence* sequence = this->sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = this->event;
	MidiFileTrack_t track = MidiFileEvent_getTrack(event);
	MidiFileEvent_delete(event);
	sequence->RefreshData();
}

MarkerEventTimeCell::MarkerEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString MarkerEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

void MarkerEventTimeCell::SmallIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
    Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = this->row->event;
	long tick = MidiFileEvent_getTick(event);
	long new_tick = tick + sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(tick));
	MidiFileEvent_setTick(event, new_tick);
	sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));
}

void MarkerEventTimeCell::SmallDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
    Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = this->row->event;
	long tick = MidiFileEvent_getTick(event);
	long new_tick = std::max<long>(tick - sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(tick)), 0);
	MidiFileEvent_setTick(event, new_tick);
	sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));
}

void MarkerEventTimeCell::LargeIncrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
    Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = this->row->event;
	long tick = MidiFileEvent_getTick(event);
	long step_number = sequence_editor->GetStepNumberFromTick(tick);
	long step_tick = sequence_editor->step_size->GetTickFromStep(step_number);
	long new_step_tick = sequence_editor->step_size->GetTickFromStep(step_number + 1);
	long new_tick = new_step_tick + (tick - step_tick);
	MidiFileEvent_setTick(event, new_tick);
	sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));
}

void MarkerEventTimeCell::LargeDecrease()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
    Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = this->row->event;
	long tick = MidiFileEvent_getTick(event);
	long step_number = sequence_editor->GetStepNumberFromTick(tick);
	long step_tick = sequence_editor->step_size->GetTickFromStep(step_number);
	long new_step_tick = sequence_editor->step_size->GetTickFromStep(std::max<long>(step_number - 1, 0));
	long new_tick = new_step_tick + (tick - step_tick);
	MidiFileEvent_setTick(event, new_tick);
	sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));
}

void MarkerEventTimeCell::Quantize()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
    Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = this->row->event;
	long tick = MidiFileEvent_getTick(event);
	long new_tick = sequence_editor->step_size->GetTickFromStep(sequence_editor->GetStepNumberFromTick(tick));
	MidiFileEvent_setTick(event, new_tick);
	sequence->RefreshData();
	sequence_editor->SetCurrentRowNumber(sequence_editor->GetRowNumberForEvent(event));
}

MarkerEventNameCell::MarkerEventNameCell(Row* row): Cell(row)
{
	this->label = wxString("Name");
}

wxString MarkerEventNameCell::GetValueText()
{
	return wxString(MidiFileMarkerEvent_getText(this->row->event));
}

void MarkerEventNameCell::EnterValue()
{
	SequenceEditor* sequence_editor = this->row->sequence_editor;
	MidiFileEvent_t event = this->row->event;
	wxTextEntryDialog* dialog = new wxTextEntryDialog(sequence_editor, "Name", "Edit Marker", MidiFileMarkerEvent_getText(event));

	if (dialog->ShowModal() == wxID_OK)
	{
    	Sequence* sequence = sequence_editor->sequence;
		sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
		wxString name = wxString(MidiFileMarkerEvent_getText(event));
		wxString new_name = dialog->GetValue();
		MidiFileMarkerEvent_setText(event, (char *)(new_name.ToStdString().c_str()));
		sequence->RefreshData();
	}

	dialog->Destroy();
}


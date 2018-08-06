
#include <wx/wx.h>
#include <midifile.h>
#include <midifile-extensions.h>
#include "marker-event.h"
#include "sequence-editor.h"

MarkerEventType* MarkerEventType::GetInstance()
{
	static MarkerEventType* instance = new MarkerEventType();
	return instance;
}

MarkerEventType::MarkerEventType()
{
	this->name = wxString("Marker");
	this->short_name = wxString("Marker");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new MarkerEventTimeCell();
	this->cells[2] = new Cell();
	this->cells[3] = new Cell();
	this->cells[4] = new MarkerEventNameCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool MarkerEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isMarkerEvent(event);
}

MarkerEventTimeCell::MarkerEventTimeCell()
{
	this->label = wxString("Time");
}

wxString MarkerEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

void MarkerEventTimeCell::SmallIncrease(SequenceEditor* sequence_editor, Row* row)
{
	Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = row->event;
	long tick = MidiFileEvent_getTick(event);
	long new_tick = tick + sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(tick));
	MidiFileEvent_setTickWithAnnotations(event, new_tick);
}

void MarkerEventTimeCell::SmallDecrease(SequenceEditor* sequence_editor, Row* row)
{
	Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = row->event;
	long tick = MidiFileEvent_getTick(event);
	long new_tick = std::max<long>(tick - sequence_editor->GetNumberOfTicksPerPixel(sequence_editor->GetStepNumberFromTick(tick)), 0);
	MidiFileEvent_setTickWithAnnotations(event, new_tick);
}

void MarkerEventTimeCell::LargeIncrease(SequenceEditor* sequence_editor, Row* row)
{
	Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = row->event;
	long tick = MidiFileEvent_getTick(event);
	long step_number = sequence_editor->GetStepNumberFromTick(tick);
	long step_tick = sequence_editor->step_size->GetTickFromStep(step_number);
	long new_step_tick = sequence_editor->step_size->GetTickFromStep(step_number + 1);
	long new_tick = new_step_tick + (tick - step_tick);
	MidiFileEvent_setTickWithAnnotations(event, new_tick);
}

void MarkerEventTimeCell::LargeDecrease(SequenceEditor* sequence_editor, Row* row)
{
	Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = row->event;
	long tick = MidiFileEvent_getTick(event);
	long step_number = sequence_editor->GetStepNumberFromTick(tick);
	long step_tick = sequence_editor->step_size->GetTickFromStep(step_number);
	long new_step_tick = sequence_editor->step_size->GetTickFromStep(std::max<long>(step_number - 1, 0));
	long new_tick = new_step_tick + (tick - step_tick);
	MidiFileEvent_setTickWithAnnotations(event, new_tick);
}

void MarkerEventTimeCell::Quantize(SequenceEditor* sequence_editor, Row* row)
{
	Sequence* sequence = sequence_editor->sequence;
	sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
	MidiFileEvent_t event = row->event;
	long tick = MidiFileEvent_getTick(event);
	long new_tick = sequence_editor->step_size->GetTickFromStep(sequence_editor->GetStepNumberFromTick(tick));
	MidiFileEvent_setTickWithAnnotations(event, new_tick);
}

MarkerEventNameCell::MarkerEventNameCell()
{
	this->label = wxString("Name");
}

wxString MarkerEventNameCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString(MidiFileMarkerEvent_getText(row->event));
}

void MarkerEventNameCell::EnterValue(SequenceEditor* sequence_editor, Row* row)
{
	MidiFileEvent_t event = row->event;
	wxTextEntryDialog* dialog = new wxTextEntryDialog(sequence_editor, "Name", "Edit Marker", MidiFileMarkerEvent_getText(event));

	if (dialog->ShowModal() == wxID_OK)
	{
		Sequence* sequence = sequence_editor->sequence;
		sequence->undo_command_processor->Submit(new UndoSnapshot(sequence));
		wxString name = wxString(MidiFileMarkerEvent_getText(event));
		wxString new_name = dialog->GetValue();
		MidiFileMarkerEvent_setText(event, (char *)(new_name.ToStdString().c_str()));
	}

	dialog->Destroy();
}

void SequenceEditor::InsertMarker()
{
	this->sequence->undo_command_processor->Submit(new UndoSnapshot(this->sequence));
    Row* row = this->GetRow(this->current_row_number);
    this->PrepareRowForInsert(row);
	MidiFileTrack_t track = MidiFile_getTrackByNumber(this->sequence->midi_file, 0, 1);
	MidiFileEvent_t event = MidiFileTrack_createMarkerEvent(track, this->step_size->GetTickFromStep(row->step_number), (char *)(""));
	this->sequence->RefreshData();
	this->SetCurrentRowNumber(this->GetRowNumberForEvent(event));
}


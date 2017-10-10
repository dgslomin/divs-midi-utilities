
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "time-signature-event.h"

TimeSignatureEventType::TimeSignatureEventType()
{
	this->name = wxString("Time signature");
}

bool TimeSignatureEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isTimeSignatureEvent(event);
}

Row* TimeSignatureEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new TimeSignatureEventRow(sequence_editor, step_number, event);
}

TimeSignatureEventRow::TimeSignatureEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Ctrl");
	this->cells[0] = new TimeSignatureEventTimeCell(this);
	this->cells[1] = new Cell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new TimeSignatureEventTimeSignatureCell(this);
	this->cells[4] = new Cell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

TimeSignatureEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString TimeSignatureEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

TimeSignatureEventTimeSignatureCell(Row* row): Cell(row)
{
	this->label = wxString("Time signature");
}

wxString TimeSignatureEventTimeSignatureCell::GetValueText()
{
	return wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(this->row->event), MidiFileTimeSignatureEvent_getDenominator(this->row->event));
}



#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "time-signature-event.h"

TimeSignatureEventType* TimeSignatureEventType::GetInstance()
{
	static TimeSignatureEventType* instance = new TimeSignatureEventType();
	return instance;
}

TimeSignatureEventType::TimeSignatureEventType()
{
	this->name = wxString("Time signature");
	this->short_name = wxString("Time");
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
	this->event_type = TimeSignatureEventType::GetInstance();
	this->cells[0] = new EventTypeCell(this);
	this->cells[1] = new TimeSignatureEventTimeCell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new Cell(this);
	this->cells[4] = new TimeSignatureEventTimeSignatureCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
	this->cells[7] = new Cell(this);
}

TimeSignatureEventTimeCell::TimeSignatureEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString TimeSignatureEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

TimeSignatureEventTimeSignatureCell::TimeSignatureEventTimeSignatureCell(Row* row): Cell(row)
{
	this->label = wxString("Time signature");
}

wxString TimeSignatureEventTimeSignatureCell::GetValueText()
{
	return wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(this->row->event), MidiFileTimeSignatureEvent_getDenominator(this->row->event));
}


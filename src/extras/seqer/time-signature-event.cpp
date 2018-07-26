
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
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new TimeSignatureEventTimeCell();
	this->cells[2] = new Cell();
	this->cells[3] = new Cell();
	this->cells[4] = new TimeSignatureEventTimeSignatureCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool TimeSignatureEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isTimeSignatureEvent(event);
}

TimeSignatureEventTimeCell::TimeSignatureEventTimeCell()
{
	this->label = wxString("Time");
}

wxString TimeSignatureEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

TimeSignatureEventTimeSignatureCell::TimeSignatureEventTimeSignatureCell()
{
	this->label = wxString("Time signature");
}

wxString TimeSignatureEventTimeSignatureCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(row->event), MidiFileTimeSignatureEvent_getDenominator(row->event));
}


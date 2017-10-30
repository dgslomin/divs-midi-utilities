
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "tempo-event.h"

TempoEventType* TempoEventType::GetInstance()
{
	static TempoEventType* instance = new TempoEventType();
	return instance;
}

TempoEventType::TempoEventType()
{
	this->name = wxString("Tempo");
	this->short_name = wxString("Tempo");
}

bool TempoEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isTempoEvent(event);
}

Row* TempoEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new TempoEventRow(sequence_editor, step_number, event);
}

TempoEventRow::TempoEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->event_type = TempoEventType::GetInstance();
	this->cells[0] = new EventTypeCell(this);
	this->cells[1] = new TempoEventTimeCell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new Cell(this);
	this->cells[4] = new TempoEventTempoCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
	this->cells[7] = new Cell(this);
}

TempoEventTimeCell::TempoEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString TempoEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

TempoEventTempoCell::TempoEventTempoCell(Row* row): Cell(row)
{
	this->label = wxString("Tempo");
}

wxString TempoEventTempoCell::GetValueText()
{
	return wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(this->row->event));
}


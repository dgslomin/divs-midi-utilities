
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "tempo-event.h"

TempoEventRow::TempoEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Tempo");
	this->cells[0] = new TempoEventTimeCell(this);
	this->cells[1] = new Cell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new TempoEventTempoCell(this);
	this->cells[4] = new Cell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

TempoEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString TempoEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

TempoEventTempoCell(Row* row): Cell(row)
{
	this->label = wxString("Tempo");
}

wxString TempoEventTempoCell::GetValueText()
{
	return wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(this->row->event));
}


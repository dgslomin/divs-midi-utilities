
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "key-signature-event.h"

KeySignatureEventRow::KeySignatureEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Key");
	this->cells[0] = new KeySignatureEventTimeCell(this);
	this->cells[1] = new Cell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new KeySignatureEventKeySignatureCell(this);
	this->cells[4] = new Cell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

KeySignatureEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString KeySignatureEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

KeySignatureEventKeySignatureCell(Row* row): Cell(row)
{
	this->label = wxString("Key signature");
}

wxString KeySignatureEventKeySignatureCell::GetValueText()
{
	return GetKeyNameFromNumber(MidiFileKeySignatureEvent_getNumber(this->row->event), (bool)(MidiFileKeySignatureEvent_isMinor(this->row->event)));
}


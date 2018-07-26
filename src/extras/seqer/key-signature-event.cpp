
#include <wx/wx.h>
#include <midifile.h>
#include "key-signature-event.h"
#include "sequence-editor.h"

KeySignatureEventType* KeySignatureEventType::GetInstance()
{
	static KeySignatureEventType* instance = new KeySignatureEventType();
	return instance;
}

KeySignatureEventType::KeySignatureEventType()
{
	this->name = wxString("Key signature");
	this->short_name = wxString("Key");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new KeySignatureEventTimeCell();
	this->cells[2] = new Cell();
	this->cells[3] = new Cell();
	this->cells[4] = new KeySignatureEventKeySignatureCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool KeySignatureEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isKeySignatureEvent(event);
}

KeySignatureEventTimeCell::KeySignatureEventTimeCell()
{
	this->label = wxString("Time");
}

wxString KeySignatureEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

KeySignatureEventKeySignatureCell::KeySignatureEventKeySignatureCell()
{
	this->label = wxString("Key signature");
}

wxString KeySignatureEventKeySignatureCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return GetKeyNameFromNumber(MidiFileKeySignatureEvent_getNumber(row->event), (bool)(MidiFileKeySignatureEvent_isMinor(row->event)));
}


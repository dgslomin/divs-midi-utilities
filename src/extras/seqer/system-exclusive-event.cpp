
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "system-exclusive-event.h"

SystemExclusiveEventType* SystemExclusiveEventType::GetInstance()
{
	static SystemExclusiveEventType* instance = new SystemExclusiveEventType();
	return instance;
}

SystemExclusiveEventType::SystemExclusiveEventType()
{
	this->name = wxString("System exclusive");
	this->short_name = wxString("Sysex");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new SystemExclusiveEventTimeCell();
	this->cells[2] = new SystemExclusiveEventTrackCell();
	this->cells[3] = new Cell();
	this->cells[4] = new Cell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool SystemExclusiveEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_SYSEX);
}

SystemExclusiveEventTimeCell::SystemExclusiveEventTimeCell()
{
	this->label = wxString("Time");
}

wxString SystemExclusiveEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

SystemExclusiveEventTrackCell::SystemExclusiveEventTrackCell()
{
	this->label = wxString("Track");
}

wxString SystemExclusiveEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}


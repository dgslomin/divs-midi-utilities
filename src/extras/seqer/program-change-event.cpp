
#include <wx/wx.h>
#include <midifile.h>
#include "program-change-event.h"
#include "sequence-editor.h"

ProgramChangeEventType* ProgramChangeEventType::GetInstance()
{
	static ProgramChangeEventType* instance = new ProgramChangeEventType();
	return instance;
}

ProgramChangeEventType::ProgramChangeEventType()
{
	this->name = wxString("Program change");
	this->short_name = wxString("Prog");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new ProgramChangeEventTimeCell();
	this->cells[2] = new ProgramChangeEventTrackCell();
	this->cells[3] = new ProgramChangeEventChannelCell();
	this->cells[4] = new ProgramChangeEventNumberCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool ProgramChangeEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE);
}

ProgramChangeEventTimeCell::ProgramChangeEventTimeCell()
{
	this->label = wxString("Time");
}

wxString ProgramChangeEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

ProgramChangeEventTrackCell::ProgramChangeEventTrackCell()
{
	this->label = wxString("Track");
}

wxString ProgramChangeEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

ProgramChangeEventChannelCell::ProgramChangeEventChannelCell()
{
	this->label = wxString("Channel");
}

wxString ProgramChangeEventChannelCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileProgramChangeEvent_getChannel(row->event) + 1);
}

ProgramChangeEventNumberCell::ProgramChangeEventNumberCell()
{
	this->label = wxString("Number");
}

wxString ProgramChangeEventNumberCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileProgramChangeEvent_getNumber(row->event) + 1);
}


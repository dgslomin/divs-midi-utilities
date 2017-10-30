
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
}

bool ProgramChangeEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE);
}

Row* ProgramChangeEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new ProgramChangeEventRow(sequence_editor, step_number, event);
}

ProgramChangeEventRow::ProgramChangeEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->event_type = ProgramChangeEventType::GetInstance();
	this->cells[0] = new EventTypeCell(this);
	this->cells[1] = new ProgramChangeEventTimeCell(this);
	this->cells[2] = new ProgramChangeEventTrackCell(this);
	this->cells[3] = new ProgramChangeEventChannelCell(this);
	this->cells[4] = new ProgramChangeEventNumberCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
	this->cells[7] = new Cell(this);
}

ProgramChangeEventTimeCell::ProgramChangeEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString ProgramChangeEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

ProgramChangeEventTrackCell::ProgramChangeEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString ProgramChangeEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

ProgramChangeEventChannelCell::ProgramChangeEventChannelCell(Row* row): Cell(row)
{
	this->label = wxString("Channel");
}

wxString ProgramChangeEventChannelCell::GetValueText()
{
	return wxString::Format("%d", MidiFileProgramChangeEvent_getChannel(this->row->event) + 1);
}

ProgramChangeEventNumberCell::ProgramChangeEventNumberCell(Row* row): Cell(row)
{
	this->label = wxString("Number");
}

wxString ProgramChangeEventNumberCell::GetValueText()
{
	return wxString::Format("%d", MidiFileProgramChangeEvent_getNumber(this->row->event) + 1);
}


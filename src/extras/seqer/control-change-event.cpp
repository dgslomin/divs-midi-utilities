
#include <wx/wx.h>
#include <midifile.h>
#include "control-change-event.h"
#include "sequence-editor.h"

ControlChangeEventType::ControlChangeEventType()
{
	this->name = wxString("Control change");
}

bool ControlChangeEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE);
}

Row* ControlChangeEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new ControlChangeEventRow(sequence_editor, step_number, event);
}

ControlChangeEventRow::ControlChangeEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Ctrl");
	this->cells[0] = new ControlChangeEventTimeCell(this);
	this->cells[1] = new ControlChangeEventTrackCell(this);
	this->cells[2] = new ControlChangeEventChannelCell(this);
	this->cells[3] = new ControlChangeEventNumberCell(this);
	this->cells[4] = new ControlChangeEventValueCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

ControlChangeEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString ControlChangeEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

ControlChangeEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString ControlChangeEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

ControlChangeEventChannelCell(Row* row): Cell(row)
{
	this->label = wxString("Channel");
}

wxString ControlChangeEventChannelCell::GetValueText()
{
	return wxString::Format("%d", MidiFileControlChangeEvent_getChannel(this->row->event) + 1);
}

ControlChangeEventNumberCell(Row* row): Cell(row)
{
	this->label = wxString("Number");
}

wxString ControlChangeEventNumberCell::GetValueText()
{
	return wxString::Format("%d", MidiFileControlChangeEvent_getNumber(this->row->event) + 1);
}

ControlChangeEventValueCell(Row* row): Cell(row)
{
	this->label = wxString("Value");
}

wxString ControlChangeEventValueCell::GetValueText()
{
	return wxString::Format("%d", MidiFileControlChangeEvent_getValue(this->row->event));
}


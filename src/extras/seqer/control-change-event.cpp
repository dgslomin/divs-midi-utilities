
#include <wx/wx.h>
#include <midifile.h>
#include "control-change-event.h"
#include "sequence-editor.h"

ControlChangeEventType* ControlChangeEventType::GetInstance()
{
	static ControlChangeEventType* instance = new ControlChangeEventType();
	return instance;
}

ControlChangeEventType::ControlChangeEventType()
{
	this->name = wxString("Control change");
	this->short_name = wxString("Ctrl");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new ControlChangeEventTimeCell();
	this->cells[2] = new ControlChangeEventTrackCell();
	this->cells[3] = new ControlChangeEventChannelCell();
	this->cells[4] = new ControlChangeEventNumberCell();
	this->cells[5] = new ControlChangeEventValueCell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool ControlChangeEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE);
}

ControlChangeEventTimeCell::ControlChangeEventTimeCell()
{
	this->label = wxString("Time");
}

wxString ControlChangeEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

ControlChangeEventTrackCell::ControlChangeEventTrackCell()
{
	this->label = wxString("Track");
}

wxString ControlChangeEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

ControlChangeEventChannelCell::ControlChangeEventChannelCell()
{
	this->label = wxString("Channel");
}

wxString ControlChangeEventChannelCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileControlChangeEvent_getChannel(row->event) + 1);
}

ControlChangeEventNumberCell::ControlChangeEventNumberCell()
{
	this->label = wxString("Number");
}

wxString ControlChangeEventNumberCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileControlChangeEvent_getNumber(row->event) + 1);
}

ControlChangeEventValueCell::ControlChangeEventValueCell()
{
	this->label = wxString("Value");
}

wxString ControlChangeEventValueCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileControlChangeEvent_getValue(row->event));
}


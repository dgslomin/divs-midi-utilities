
#include <wx/wx.h>
#include <midifile.h>
#include "pitch-bend-event.h"
#include "sequence-editor.h"

PitchBendEventType::PitchBendEventType()
{
	this->name = wxString("Pitch bend");
}

bool PitchBendEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL);
}

Row* PitchBendEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new PitchBendEventRow(sequence_editor, step_number, event);
}

PitchBendEventRow::PitchBendEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Bend");
	this->cells[0] = new PitchBendEventTimeCell(this);
	this->cells[1] = new PitchBendEventTrackCell(this);
	this->cells[2] = new PitchBendEventChannelCell(this);
	this->cells[3] = new Cell(this);
	this->cells[4] = new PitchBendEventValueCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

PitchBendEventTimeCell::PitchBendEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString PitchBendEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

PitchBendEventTrackCell::PitchBendEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString PitchBendEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

PitchBendEventChannelCell::PitchBendEventChannelCell(Row* row): Cell(row)
{
	this->label = wxString("Channel");
}

wxString PitchBendEventChannelCell::GetValueText()
{
	return wxString::Format("%d", MidiFilePitchWheelEvent_getChannel(this->row->event) + 1);
}

PitchBendEventValueCell::PitchBendEventValueCell(Row* row): Cell(row)
{
	this->label = wxString("Value");
}

wxString PitchBendEventValueCell::GetValueText()
{
	return wxString::Format("%d", MidiFilePitchWheelEvent_getValue(this->row->event));
}


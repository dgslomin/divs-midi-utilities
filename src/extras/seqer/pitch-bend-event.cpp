
#include <wx/wx.h>
#include <midifile.h>
#include "pitch-bend-event.h"
#include "sequence-editor.h"

PitchBendEventType* PitchBendEventType::GetInstance()
{
	static PitchBendEventType* instance = new PitchBendEventType();
	return instance;
}

PitchBendEventType::PitchBendEventType()
{
	this->name = wxString("Pitch bend");
	this->short_name = wxString("Bend");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new PitchBendEventTimeCell();
	this->cells[2] = new PitchBendEventTrackCell();
	this->cells[3] = new PitchBendEventChannelCell();
	this->cells[4] = new Cell();
	this->cells[5] = new PitchBendEventValueCell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool PitchBendEventType::MatchesEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL);
}

PitchBendEventTimeCell::PitchBendEventTimeCell()
{
	this->label = wxString("Time");
}

wxString PitchBendEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

PitchBendEventTrackCell::PitchBendEventTrackCell()
{
	this->label = wxString("Track");
}

wxString PitchBendEventTrackCell::GetValueText(SequenceEditor* sequence_Editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

PitchBendEventChannelCell::PitchBendEventChannelCell()
{
	this->label = wxString("Channel");
}

wxString PitchBendEventChannelCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFilePitchWheelEvent_getChannel(row->event) + 1);
}

PitchBendEventValueCell::PitchBendEventValueCell()
{
	this->label = wxString("Value");
}

wxString PitchBendEventValueCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFilePitchWheelEvent_getValue(row->event));
}


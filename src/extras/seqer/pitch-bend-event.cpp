
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "pitch-bend-event.h"

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

PitchBendEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString PitchBendEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

PitchBendEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString PitchBendEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

PitchBendEventChannelCell(Row* row): Cell(row)
{
	this->label = wxString("Channel");
}

wxString PitchBendEventChannelCell::GetValueText()
{
	return wxString::Format("%d", MidiFilePitchBendEvent_getChannel(this->row->event) + 1);
}

PitchBendEventValueCell(Row* row): Cell(row)
{
	this->label = wxString("Value");
}

wxString PitchBendEventValueCell::GetValueText()
{
	return wxString::Format("%d", MidiFilePitchBendEvent_getValue(this->row->event));
}



#include <wx/wx.h>
#include <midifile.h>
#include "port-event.h"
#include "sequence-editor.h"

PortEventType::PortEventType()
{
	this->name = wxString("Port");
}

bool PortEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isPortEvent(event);
}

Row* PortEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new PortEventRow(sequence_editor, step_number, event);
}

PortEventRow::PortEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Port");
	this->cells[0] = new PortEventTimeCell(this);
	this->cells[1] = new PortEventTrackCell(this);
	this->cells[2] = new Cell(this);
	this->cells[3] = new PortEventNameCell(this);
	this->cells[4] = new Cell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

PortEventTimeCell::PortEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString PortEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

PortEventTrackCell::PortEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString PortEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

PortEventNameCell::PortEventNameCell(Row* row): Cell(row)
{
	this->label = wxString("Name");
}

wxString PortEventNameCell::GetValueText()
{
	return wxString(MidiFilePortEvent_getName(this->row->event));
}


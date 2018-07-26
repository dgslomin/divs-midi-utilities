
#include <wx/wx.h>
#include <midifile.h>
#include "port-event.h"
#include "sequence-editor.h"

PortEventType* PortEventType::GetInstance()
{
	static PortEventType* instance = new PortEventType();
	return instance;
}

PortEventType::PortEventType()
{
	this->name = wxString("Port");
	this->short_name = wxString("Port");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new PortEventTimeCell();
	this->cells[2] = new PortEventTrackCell();
	this->cells[3] = new Cell();
	this->cells[4] = new PortEventNameCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool PortEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isPortEvent(event);
}

PortEventTimeCell::PortEventTimeCell()
{
	this->label = wxString("Time");
}

wxString PortEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

PortEventTrackCell::PortEventTrackCell()
{
	this->label = wxString("Track");
}

wxString PortEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

PortEventNameCell::PortEventNameCell()
{
	this->label = wxString("Name");
}

wxString PortEventNameCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString(MidiFilePortEvent_getName(row->event));
}


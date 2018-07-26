
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "tempo-event.h"

TempoEventType* TempoEventType::GetInstance()
{
	static TempoEventType* instance = new TempoEventType();
	return instance;
}

TempoEventType::TempoEventType()
{
	this->name = wxString("Tempo");
	this->short_name = wxString("Tempo");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new TempoEventTimeCell();
	this->cells[2] = new Cell();
	this->cells[3] = new Cell();
	this->cells[4] = new TempoEventTempoCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool TempoEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isTempoEvent(event);
}

TempoEventTimeCell::TempoEventTimeCell()
{
	this->label = wxString("Time");
}

wxString TempoEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

TempoEventTempoCell::TempoEventTempoCell()
{
	this->label = wxString("Tempo");
}

wxString TempoEventTempoCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(row->event));
}


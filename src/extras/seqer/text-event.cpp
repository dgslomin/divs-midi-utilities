
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "text-event.h"

TextEventType* TextEventType::GetInstance()
{
	static TextEventType* instance = new TextEventType();
	return instance;
}

TextEventType::TextEventType()
{
	this->name = wxString("Text");
	this->short_name = wxString("Text");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new TextEventTimeCell();
	this->cells[2] = new TextEventTrackCell();
	this->cells[3] = new Cell();
	this->cells[4] = new TextEventTextCell();
	this->cells[5] = new Cell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool TextEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isTextEvent(event);
}

TextEventTimeCell::TextEventTimeCell()
{
	this->label = wxString("Time");
}

wxString TextEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

TextEventTrackCell::TextEventTrackCell()
{
	this->label = wxString("Track");
}

wxString TextEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

TextEventTextCell::TextEventTextCell()
{
	this->label = wxString("Text");
}

wxString TextEventTextCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString(MidiFileTextEvent_getText(row->event));
}


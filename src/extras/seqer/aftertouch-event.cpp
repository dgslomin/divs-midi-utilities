
#include <wx/wx.h>
#include <midifile.h>
#include "aftertouch-event.h"
#include "sequence-editor.h"

AftertouchEventType* AftertouchEventType::GetInstance()
{
	static AftertouchEventType* instance = new AftertouchEventType();
	return instance;
}

AftertouchEventType::AftertouchEventType()
{
	this->name = wxString("Aftertouch");
	this->short_name = wxString("Touch");
	this->cells[0] = new EventTypeCell();
	this->cells[1] = new AftertouchEventTimeCell();
	this->cells[2] = new AftertouchEventTrackCell();
	this->cells[3] = new AftertouchEventChannelCell();
	this->cells[4] = new AftertouchEventNoteCell();
	this->cells[5] = new AftertouchEventAmountCell();
	this->cells[6] = new Cell();
	this->cells[7] = new Cell();
}

bool AftertouchEventType::MatchesEvent(MidiFileEvent_t event)
{
	return MidiFileEvent_isPressureEvent(event);
}

AftertouchEventTimeCell::AftertouchEventTimeCell()
{
	this->label = wxString("Time");
}

wxString AftertouchEventTimeCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(row->event));
}

AftertouchEventTrackCell::AftertouchEventTrackCell()
{
	this->label = wxString("Track");
}

wxString AftertouchEventTrackCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(row->event)));
}

AftertouchEventChannelCell::AftertouchEventChannelCell()
{
	this->label = wxString("Channel");
}

wxString AftertouchEventChannelCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFilePressureEvent_getChannel(row->event) + 1);
}

AftertouchEventNoteCell::AftertouchEventNoteCell()
{
	this->label = wxString("Note");
}

wxString AftertouchEventNoteCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	if (MidiFileEvent_getType(row->event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
	{
		return GetNoteNameFromNumber(MidiFileKeyPressureEvent_getNote(row->event));
	}
	else
	{
		return wxString("-");
	}
}

AftertouchEventAmountCell::AftertouchEventAmountCell()
{
	this->label = wxString("Amount");
}

wxString AftertouchEventAmountCell::GetValueText(SequenceEditor* sequence_editor, Row* row)
{
	return wxString::Format("%d", MidiFilePressureEvent_getAmount(row->event));
}


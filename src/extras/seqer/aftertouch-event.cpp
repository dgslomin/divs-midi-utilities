
#include <wx/wx.h>
#include <midifile.h>
#include "aftertouch-event.h"
#include "sequence-editor.h"

AftertouchEventType::AftertouchEventType()
{
	this->name = wxString("Aftertouch");
}

bool AftertouchEventType::MatchesEvent(MidiFileEvent_t event)
{
	return ((MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE) || (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE));
}

Row* AftertouchEventType::GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event)
{
	return new AftertouchEventRow(sequence_editor, step_number, event);
}

AftertouchEventRow::AftertouchEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event): Row(sequence_editor, step_number, event)
{
	this->label = wxString("Touch");
	this->cells[0] = new AftertouchEventTimeCell(this);
	this->cells[1] = new AftertouchEventTrackCell(this);
	this->cells[2] = new AftertouchEventChannelCell(this);
	this->cells[3] = new AftertouchEventNoteCell(this);
	this->cells[4] = new AftertouchEventAmountCell(this);
	this->cells[5] = new Cell(this);
	this->cells[6] = new Cell(this);
}

AftertouchEventTimeCell(Row* row): Cell(row)
{
	this->label = wxString("Time");
}

wxString AftertouchEventTimeCell::GetValueText()
{
	return this->row->sequence_editor->step_size->GetTimeStringFromTick(MidiFileEvent_getTick(this->row->event));
}

AftertouchEventTrackCell(Row* row): Cell(row)
{
	this->label = wxString("Track");
}

wxString AftertouchEventTrackCell::GetValueText()
{
	return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(this->row->event)));
}

AftertouchEventChannelCell(Row* row): Cell(row)
{
	this->label = wxString("Channel");
}

wxString AftertouchEventChannelCell::GetValueText()
{
	if (MidiFileEvent_getType(this->row->event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
	{
		return wxString::Format("%d", MidiFileKeyPressureEvent_getChannel(this->row->event) + 1);
	}
	else
	{
		return wxString::Format("%d", MidiFileChannelPressureEvent_getChannel(this->row->event) + 1);
	}
}

AftertouchEventNoteCell(Row* row): Cell(row)
{
	this->label = wxString("Note");
}

wxString AftertouchEventNoteCell::GetValueText()
{
	if (MidiFileEvent_getType(this->row->event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
	{
		return GetNoteNameFromNumber(MidiFileKeyPressureEvent_getNote(this->row->event));
	}
	else
	{
		return wxString("-");
	}
}

AftertouchEventAmountCell(Row* row): Cell(row)
{
	this->label = wxString("Amount");
}

wxString AftertouchEventAmountCell::GetValueText()
{
	if (MidiFileEvent_getType(this->row->event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
	{
		return wxString::Format("%d", MidiFileKeyPressureEvent_getAmount(this->row->event));
	}
	else
	{
		return wxString::Format("%d", MidiFileChannelPressureEvent_getAmount(this->row->event));
	}
}


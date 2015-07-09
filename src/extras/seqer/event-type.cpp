
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "event-type.h"

EventType* event_types[] = {new NoteEventType(), new ControlChangeEventType(), new ProgramChangeEventType(), new AftertouchEventType(), new PitchBendEventType(), new SystemExclusiveEventType(), new TextEventType(), new LyricEventType(), new MarkerEventType(), new PortEventType(), new TempoEventType(), new TimeSignatureEventType(), new KeySignatureEventType(), NULL};

wxString NoteEventType::GetName()
{
	return wxString("Note");
}

bool NoteEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isNoteStartEvent(event));
}

wxString NoteEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Note");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFileNoteStartEvent_getChannel(event) + 1);
		}
		case 4:
		{
			return GetNoteNameFromNumber(MidiFileNoteStartEvent_getNote(event));
		}
		case 5:
		{
			return wxString::Format("%d", MidiFileNoteStartEvent_getVelocity(event));
		}
		case 6:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event))));
		}
		case 7:
		{
			return wxString::Format("%d", MidiFileNoteEndEvent_getVelocity(MidiFileNoteStartEvent_getNoteEndEvent(event)));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString ControlChangeEventType::GetName()
{
	return wxString("Control change");
}

bool ControlChangeEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE;
}

wxString ControlChangeEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Ctrl");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFileControlChangeEvent_getChannel(event) + 1);
		}
		case 4:
		{
			return wxString::Format("%d", MidiFileControlChangeEvent_getNumber(event) + 1);
		}
		case 5:
		{
			return wxString::Format("%d", MidiFileControlChangeEvent_getValue(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString ProgramChangeEventType::GetName()
{
	return wxString("Program change");
}

bool ProgramChangeEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE;
}

wxString ProgramChangeEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Prog");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFileProgramChangeEvent_getChannel(event) + 1);
		}
		case 4:
		{
			return wxString::Format("%d", MidiFileProgramChangeEvent_getNumber(event) + 1);
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString AftertouchEventType::GetName()
{
	return wxString("Aftertouch");
}

bool AftertouchEventType::Matches(MidiFileEvent_t event)
{
	return (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE) || (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE);
}

wxString AftertouchEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Touch");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
			{
				return wxString::Format("%d", MidiFileKeyPressureEvent_getChannel(event) + 1);
			}
			else
			{
				return wxString::Format("%d", MidiFileChannelPressureEvent_getChannel(event) + 1);
			}
		}
		case 4:
		{
			if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
			{
				return GetNoteNameFromNumber(MidiFileKeyPressureEvent_getNote(event));
			}
			else
			{
				return wxString("-");
			}
		}
		case 5:
		{
			if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE)
			{
				return wxString::Format("%d", MidiFileKeyPressureEvent_getAmount(event));
			}
			else
			{
				return wxString::Format("%d", MidiFileChannelPressureEvent_getAmount(event));
			}
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString PitchBendEventType::GetName()
{
	return wxString("Pitch bend");
}

bool PitchBendEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL;
}

wxString PitchBendEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Bend");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 3:
		{
			return wxString::Format("%d", MidiFilePitchWheelEvent_getChannel(event) + 1);
		}
		case 5:
		{
			return wxString::Format("%d", MidiFilePitchWheelEvent_getValue(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString SystemExclusiveEventType::GetName()
{
	return wxString("System exclusive");
}

bool SystemExclusiveEventType::Matches(MidiFileEvent_t event)
{
	return MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_SYSEX;
}

wxString SystemExclusiveEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	// TODO: render bytes as ascii plus hex escapes

	switch (column_number)
	{
		case 0:
		{
			return wxString("Sysex");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString TextEventType::GetName()
{
	return wxString("Text");
}

bool TextEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isTextEvent(event));
}

wxString TextEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Text");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFileTextEvent_getText(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString LyricEventType::GetName()
{
	return wxString("Lyric");
}

bool LyricEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isLyricEvent(event));
}

wxString LyricEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Lyric");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFileLyricEvent_getText(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString MarkerEventType::GetName()
{
	return wxString("Marker");
}

bool MarkerEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isMarkerEvent(event));
}

wxString MarkerEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Marker");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFileMarkerEvent_getText(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString PortEventType::GetName()
{
	return wxString("Port");
}

bool PortEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isPortEvent(event));
}

wxString PortEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Port");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString(MidiFilePortEvent_getName(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString TempoEventType::GetName()
{
	return wxString("Tempo");
}

bool TempoEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isTempoEvent(event));
}

wxString TempoEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Tempo");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString::Format("%5.3f", MidiFileTempoEvent_getTempo(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString TimeSignatureEventType::GetName()
{
	return wxString("Time signature");
}

bool TimeSignatureEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isTimeSignatureEvent(event));
}

wxString TimeSignatureEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Time");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return wxString::Format("%d/%d", MidiFileTimeSignatureEvent_getNumerator(event), MidiFileTimeSignatureEvent_getDenominator(event));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

wxString KeySignatureEventType::GetName()
{
	return wxString("Key signature");
}

bool KeySignatureEventType::Matches(MidiFileEvent_t event)
{
	return (bool)(MidiFileEvent_isKeySignatureEvent(event));
}

wxString KeySignatureEventType::GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number)
{
	switch (column_number)
	{
		case 0:
		{
			return wxString("Key");
		}
		case 1:
		{
			return wxString(MidiFile_getMeasureBeatStringFromTick(event_list->sequence_editor->sequence->midi_file, MidiFileEvent_getTick(event)));
		}
		case 2:
		{
			return wxString::Format("%d", MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)));
		}
		case 4:
		{
			return GetKeyNameFromNumber(MidiFileKeySignatureEvent_getNumber(event), (bool)(MidiFileKeySignatureEvent_isMinor(event)));
		}
		default:
		{
			return wxEmptyString;
		}
	}
}


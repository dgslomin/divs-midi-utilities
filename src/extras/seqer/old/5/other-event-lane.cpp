
#include <wx/wx.h>

void OtherEventLane::GetLabels()
{
	for (MidiFileEvent_t midi_event = MidiFile_getFirstEventInFile(this->window->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		char *label = NULL;

		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			label = "Note";
		}
		else if ((MidiFileEvent_isNoteOffEvent(midi_event) && (MidiFileEvent_getNoteStartEvent(midi_event) == NULL))
		{
			label = "Off";
		}
		else if (MidiFileEvent_isTextEvent(midi_event))
		{
			label = "Text";
		}
		else if (MidiFileEvent_isLyricEvent(midi_event))
		{
			label = "Lyric";
		}
		else if (MidiFileEvent_isMarkerEvent(midi_event))
		{
			label = "Marker";
		}
		else if (MidiFileEvent_isTempoEvent(midi_event))
		{
			label = "Tempo";
		}
		else if (MidiFileEvent_isTimeSignatureEvent(midi_event))
		{
			label = "Meter";
		}
		else if (MidiFileEvent_isKeySignatureEvent(midi_event))
		{
			label = "Key";
		}
		else
		{
			switch (MidiFileEvent_getType(midi_event))
			{
				case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
				{
					label = "Pressure";
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
				{
					label = "Control";
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
				{
					label = "Program";
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
				{
					label = "Pressure";
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
				{
					label = "Bend";
					break;
				}
				case MIDI_FILE_EVENT_TYPE_SYSEX:
				{
					label = "Sysex";
					break;
				}
				case MIDI_FILE_EVENT_TYPE_META:
				{
					label = "Meta";
					break;
				}
			}
		}
	}
}


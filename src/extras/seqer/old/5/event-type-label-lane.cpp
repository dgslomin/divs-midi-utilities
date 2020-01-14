
#include <wx/wx.h>
#include "label.h"
#include "event-type-label-lane.h"
#include "midifile.h"

void EventTypeLabelLane::PopulateLabels()
{
	this->labels.clear();

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEventInFile(this->window->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		wxString label = NULL;

		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			label = wxString("Note");
		}
		else if ((MidiFileEvent_isNoteOffEvent(midi_event) && (MidiFileEvent_getNoteStartEvent(midi_event) == NULL))
		{
			label = wxString("Off");
		}
		else if (MidiFileEvent_isTextEvent(midi_event))
		{
			label = wxString("Text");
		}
		else if (MidiFileEvent_isLyricEvent(midi_event))
		{
			label = wxString("Lyric");
		}
		else if (MidiFileEvent_isMarkerEvent(midi_event))
		{
			label = wxString("Marker");
		}
		else if (MidiFileEvent_isTempoEvent(midi_event))
		{
			label = wxString("Tempo");
		}
		else if (MidiFileEvent_isTimeSignatureEvent(midi_event))
		{
			label = wxString("Meter");
		}
		else if (MidiFileEvent_isKeySignatureEvent(midi_event))
		{
			label = wxString("Key");
		}
		else
		{
			switch (MidiFileEvent_getType(midi_event))
			{
				case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
				{
					label = wxString("Pressure");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
				{
					label = wxString("Control");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
				{
					label = wxString("Program");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
				{
					label = wxString("Pressure");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
				{
					label = wxString("Bend");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_SYSEX:
				{
					label = wxString("Sysex");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_META:
				{
					label = wxString("Meta");
					break;
				}
			}
		}

		if (label != NULL) this->labels.push_back(Label(midi_event, label));
	}
}


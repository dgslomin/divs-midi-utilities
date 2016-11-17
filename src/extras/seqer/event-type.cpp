
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "step-size.h"
#include "event-type.h"

wxString SeqerEventType_getName(SeqerEventType_t event_type)
{
	switch (event_type)
	{
		case SEQER_EVENT_TYPE_NOTE:
		{
			return wxString("Note");
		}
		case SEQER_EVENT_TYPE_CONTROL_CHANGE:
		{
			return wxString("Control change");
		}
		case SEQER_EVENT_TYPE_PROGRAM_CHANGE:
		{
			return wxString("Program change");
		}
		case SEQER_EVENT_TYPE_AFTERTOUCH:
		{
			return wxString("Aftertouch");
		}
		case SEQER_EVENT_TYPE_PITCH_BEND:
		{
			return wxString("Pitch bend");
		}
		case SEQER_EVENT_TYPE_SYSTEM_EXCLUSIVE:
		{
			return wxString("System exclusive");
		}
		case SEQER_EVENT_TYPE_TEXT:
		{
			return wxString("Text");
		}
		case SEQER_EVENT_TYPE_LYRIC:
		{
			return wxString("Lyric");
		}
		case SEQER_EVENT_TYPE_MARKER:
		{
			return wxString("Marker");
		}
		case SEQER_EVENT_TYPE_PORT:
		{
			return wxString("Port");
		}
		case SEQER_EVENT_TYPE_TEMPO:
		{
			return wxString("Tempo");
		}
		case SEQER_EVENT_TYPE_TIME_SIGNATURE:
		{
			return wxString("Time signature");
		}
		case SEQER_EVENT_TYPE_KEY_SIGNATURE:
		{
			return wxString("Key signature");
		}
		default:
		{
			return wxEmptyString;
		}
	}
}

SeqerEventType_t SeqerEvent_getType(MidiFileEvent_t event)
{
	if (MidiFileEvent_isNoteStartEvent(event)) return SEQER_EVENT_TYPE_NOTE;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) return SEQER_EVENT_TYPE_CONTROL_CHANGE;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE) return SEQER_EVENT_TYPE_PROGRAM_CHANGE;
	if ((MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_KEY_PRESSURE) || (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE)) return SEQER_EVENT_TYPE_AFTERTOUCH;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_PITCH_WHEEL) return SEQER_EVENT_TYPE_PITCH_BEND;
	if (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_SYSEX) return SEQER_EVENT_TYPE_SYSTEM_EXCLUSIVE;
	if (MidiFileEvent_isTextEvent(event)) return SEQER_EVENT_TYPE_TEXT;
	if (MidiFileEvent_isLyricEvent(event)) return SEQER_EVENT_TYPE_LYRIC;
	if (MidiFileEvent_isMarkerEvent(event)) return SEQER_EVENT_TYPE_MARKER;
	if (MidiFileEvent_isPortEvent(event)) return SEQER_EVENT_TYPE_PORT;
	if (MidiFileEvent_isTempoEvent(event)) return SEQER_EVENT_TYPE_TEMPO;
	if (MidiFileEvent_isTimeSignatureEvent(event)) return SEQER_EVENT_TYPE_TIME_SIGNATURE;
	if (MidiFileEvent_isKeySignatureEvent(event)) return SEQER_EVENT_TYPE_KEY_SIGNATURE;
	return SEQER_EVENT_TYPE_INVALID;
}


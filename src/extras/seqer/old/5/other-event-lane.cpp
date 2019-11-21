
#include <wx/wx.h>

OtherEventLane::OtherEventLane(Window* window): wxWindow(window, wxID_ANY)
{
	this->Bind(wxEVT_PAINT, &OnPaint);
}

OtherEventLane::~OtherEventLane()
{
}

void OtherEventLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEventInFile(this->window->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isNoteStartEvent(midi_event))
		{
			label = wxString::Format("Note %s", GetNoteNameFromNumber(MidiFileNoteStartEvent_getNote(midi_event)));
		}
		else if ((MidiFileEvent_isNoteOffEvent(midi_event) && (MidiFileEvent_getNoteStartEvent(midi_event) == NULL))
		{
			label = wxString::Format("Off %s", GetNoteNameFromNumber(MidiFileNoteOffEvent_getNote(midi_event)));
		}
		else if (MidiFileEvent_isTextEvent(midi_event))
		{
			label = wxString::Format("Text %s", MidiFileTextEvent_getText(midi_event));
		}
		else if (MidiFileEvent_isLyricEvent(midi_event))
		{
			label = wxString::Format("Lyric %s", MidiFileLyricEvent_getText(midi_event));
		}
		else if (MidiFileEvent_isMarkerEvent(midi_event))
		{
			label = wxString::Format("Marker %s", MidiFileMarkerEvent_getText(midi_event));
		}
		else if (MidiFileEvent_isTempoEvent(midi_event))
		{
			label = wxString::Format("Tempo %d", MidiFileTempoEvent_getTempo(midi_event));
		}
		else if (MidiFileEvent_isTimeSignatureEvent(midi_event))
		{
			label = wxString::Format("Meter %d/%d", MidiFileTimeSignatureEvent_getNumerator(midi_event), MidiFileTimeSignatureEvent_getDenominator(midi_event));
		}
		else if (MidiFileEvent_isKeySignatureEvent(midi_event))
		{
			label = wxString::Format("Key %s", GetKeyNameFromNumber(MidiFileKeySignatureEvent_getNumber(midi_event), MidiFileKeySignatureEvent_isMinor(midi_event)));
		}
		else
		{
			switch (MidiFileEvent_getType(midi_event))
			{
				case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
				{
					label = wxString::Format("Pressure %s %d", MidiFileKeyPressureEvent_getNote(midi_event), MidiFileKeyPressureEvent_getAmount(midi_event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
				{
					label = wxString::Format("Control %d %d", MidiFileControlChangeEvent_getNumber(midi_event), MidiFileControlChangeEvent_getValue(midi_event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
				{
					label = wxString::Format("Program %d", MidiFileProgramChangeEvent_getNumber(midi_event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
				{
					label = wxString::Format("Pressure %d", MidiFileChannelPressureEvent_getAmount(midi_event));
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
				{
					label = wxString::Format("Bend %d", MidiFilePitchWheelEvent_getValue(midi_event));
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


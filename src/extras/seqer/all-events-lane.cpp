
#include <QtWidgets>
#include "all-events-lane.h"
#include "label-lane.h"
#include "midifile.h"
#include "window.h"

AllEventsLane::AllEventsLane(Window* window): LabelLane(window)
{
}

void AllEventsLane::populateLabels()
{
	this->labels.clear();

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		QString label_text;

		if (MidiFileEvent_isTextEvent(midi_event))
		{
			label_text = tr("Tx");
		}
		else if (MidiFileEvent_isLyricEvent(midi_event))
		{
			label_text = tr("L");
		}
		else if (MidiFileEvent_isMarkerEvent(midi_event))
		{
			label_text = tr("Mk");
		}
		else if (MidiFileEvent_isTempoEvent(midi_event))
		{
			label_text = tr("Tp");
		}
		else if (MidiFileEvent_isTimeSignatureEvent(midi_event))
		{
			label_text = tr("Ts");
		}
		else if (MidiFileEvent_isKeySignatureEvent(midi_event))
		{
			label_text = tr("Ks");
		}
		else
		{
			switch (MidiFileEvent_getType(midi_event))
			{
				case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
				{
					label_text = tr("N");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NOTE_ON:
				{
					label_text = tr("N");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
				{
					label_text = tr("P");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
				{
					label_text = tr("C");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
				{
					label_text = tr("Pc");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
				{
					label_text = tr("P");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
				{
					label_text = tr("B");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_SYSEX:
				{
					label_text = tr("Sx");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_META:
				{
					label_text = tr("Ma");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NOTE:
				{
					label_text = tr("N");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_FINE_CONTROL_CHANGE:
				{
					label_text = tr("C");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_RPN:
				{
					label_text = tr("C");
					break;
				}
				case MIDI_FILE_EVENT_TYPE_NRPN:
				{
					label_text = tr("C");
					break;
				}
				default:
				{
					break;
				}
			}
		}

		if (!label_text.isEmpty()) this->labels.append(Label(midi_event, label_text));
	}
}

MidiFileEvent_t AllEventsLane::addEventAtXY(int x, int y)
{
	// TODO
	Q_UNUSED(x)
	Q_UNUSED(y)
	return NULL;
}

void AllEventsLane::moveEventsByXY(int x_offset, int y_offset)
{
	// TODO
	Q_UNUSED(x_offset)
	Q_UNUSED(y_offset)
}


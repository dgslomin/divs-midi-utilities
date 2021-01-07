
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			printf("%-2d Note Off         %-3d %d\n", MidiUtilNoteOffMessage_getChannel(message), MidiUtilNoteOffMessage_getNote(message), MidiUtilNoteOffMessage_getVelocity(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			printf("%-2d Note On          %-3d %d\n", MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message), MidiUtilNoteOnMessage_getVelocity(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE:
		{
			printf("%-2d Key Pressure     %-3d %d\n", MidiUtilKeyPressureMessage_getChannel(message), MidiUtilKeyPressureMessage_getNote(message), MidiUtilKeyPressureMessage_getAmount(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			printf("%-2d Control Change   %-3d %d\n", MidiUtilControlChangeMessage_getChannel(message), MidiUtilControlChangeMessage_getNumber(message), MidiUtilControlChangeMessage_getValue(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE:
		{
			printf("%-2d Program Change   %d\n", MidiUtilProgramChangeMessage_getChannel(message), MidiUtilProgramChangeMessage_getNumber(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE:
		{
			printf("%-2d Channel Pressure %d\n", MidiUtilChannelPressureMessage_getChannel(message), MidiUtilChannelPressureMessage_getAmount(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL:
		{
			printf("%-2d Pitch Wheel      %d\n", MidiUtilPitchWheelMessage_getChannel(message), MidiUtilPitchWheelMessage_getValue(message));
			break;
		}
		default:
		{
			break;
		}
	}
}

int main(int argc, char **argv)
{
	RtMidiInPtr midi_in = NULL;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("dispmidi", argv[i], "dispmidi", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_in == NULL) usage(argv[0]);
	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);
	return 0;
}


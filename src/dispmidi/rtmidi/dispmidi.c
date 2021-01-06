
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (rtmidi_message_get_type(message))
	{
		case RTMIDI_MESSAGE_TYPE_NOTE_OFF:
		{
			printf("%-2d Note Off         %-3d %d\n", rtmidi_note_off_message_get_channel(message), rtmidi_note_off_message_get_note(message), rtmidi_note_off_message_get_velocity(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_NOTE_ON:
		{
			printf("%-2d Note On          %-3d %d\n", rtmidi_note_on_message_get_channel(message), rtmidi_note_on_message_get_note(message), rtmidi_note_on_message_get_velocity(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_KEY_PRESSURE:
		{
			printf("%-2d Key Pressure     %-3d %d\n", rtmidi_key_pressure_message_get_channel(message), rtmidi_key_pressure_message_get_note(message), rtmidi_key_pressure_message_get_amount(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			printf("%-2d Control Change   %-3d %d\n", rtmidi_control_change_message_get_channel(message), rtmidi_control_change_message_get_number(message), rtmidi_control_change_message_get_value(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_PROGRAM_CHANGE:
		{
			printf("%-2d Program Change   %d\n", rtmidi_program_change_message_get_channel(message), rtmidi_program_change_message_get_number(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_CHANNEL_PRESSURE:
		{
			printf("%-2d Channel Pressure %d\n", rtmidi_channel_pressure_message_get_channel(message), rtmidi_channel_pressure_message_get_amount(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_PITCH_WHEEL:
		{
			printf("%-2d Pitch Wheel      %d\n", rtmidi_pitch_wheel_message_get_channel(message), rtmidi_pitch_wheel_message_get_value(message));
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

			if ((midi_in = rtmidi_open_in_port("dispmidi", argv[i], "in", handle_midi_message, NULL)) == NULL)
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


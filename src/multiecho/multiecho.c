
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static MidiUtilAlarm_t alarm = NULL;
static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int number_of_echoes = 0;
static int echo_delay_msecs_array[128];
static int echo_note_interval_array[128];
static float echo_velocity_scaling_array[128];

static void handle_alarm(int cancelled, void *user_data)
{
	unsigned char *message = (unsigned char *)(user_data);
	if (!cancelled) rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));
	free(message);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			int echo_number;

			for (echo_number = 0; echo_number < number_of_echoes; echo_number++)
			{
				int new_note = MidiUtilNoteOffMessage_getNote(message) + echo_note_interval_array[echo_number];
				int new_velocity = MidiUtil_clamp((int)(MidiUtilNoteOffMessage_getVelocity(message) * echo_velocity_scaling_array[echo_number]), 0, 127);

				if (new_note >= 0 && new_note < 128)
				{
					unsigned char *new_message = (unsigned char *)(malloc(MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF));
					MidiUtilMessage_setNoteOff(new_message, MidiUtilNoteOffMessage_getChannel(message), new_note, new_velocity);
					MidiUtilAlarm_add(alarm, echo_delay_msecs_array[echo_number], handle_alarm, new_message);
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int echo_number;

			for (echo_number = 0; echo_number < number_of_echoes; echo_number++)
			{
				int new_note = MidiUtilNoteOnMessage_getNote(message) + echo_note_interval_array[echo_number];
				int new_velocity = MidiUtil_clamp((int)(MidiUtilNoteOnMessage_getVelocity(message) * echo_velocity_scaling_array[echo_number]), 0, 127);

				if (new_note >= 0 && new_note < 128)
				{
					unsigned char *new_message = (unsigned char *)(malloc(MIDI_UTIL_MESSAGE_SIZE_NOTE_ON));
					MidiUtilMessage_setNoteOn(new_message, MidiUtilNoteOnMessage_getChannel(message), new_note, new_velocity);
					MidiUtilAlarm_add(alarm, echo_delay_msecs_array[echo_number], handle_alarm, new_message);
				}
			}

			break;
		}
		default:
		{
			rtmidi_out_send_message(midi_out, message, message_size);
			break;
		}
	}
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --echo <delay msecs> <note interval> <velocity scaling> ] ...\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;

	alarm = MidiUtilAlarm_new();

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("multiecho", argv[i], "multiecho", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("multiecho", argv[i], "multiecho")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--echo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			echo_delay_msecs_array[number_of_echoes] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			echo_note_interval_array[number_of_echoes] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			echo_velocity_scaling_array[number_of_echoes] = atof(argv[i]);
			number_of_echoes++;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL)) usage(argv[0]);
	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	MidiUtilAlarm_free(alarm);
	return 0;
}


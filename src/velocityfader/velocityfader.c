
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int controller_number = 7;
static int controller_value = 127;
static int inverted = 0;

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
			int new_velocity = MidiUtilNoteOffMessage_getVelocity(message) * (inverted ? (127 - controller_value) : controller_value) / 128;
			MidiUtilMessage_setNoteOff(new_message, MidiUtilNoteOffMessage_getChannel(message), MidiUtilNoteOffMessage_getNote(message), new_velocity);
			rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
			int new_velocity = MidiUtilNoteOnMessage_getVelocity(message) * (inverted ? (127 - controller_value) : controller_value) / 127;
			MidiUtilMessage_setNoteOn(new_message, MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message), new_velocity);
			rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			if (MidiUtilControlChangeMessage_getNumber(message) == controller_number)
			{
				controller_value = MidiUtilControlChangeMessage_getValue(message);
			}
			else
			{
				rtmidi_out_send_message(midi_out, message, message_size);
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
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --controller <n> ] [ --inverted ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("velocityfader", argv[i], "velocityfader", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("velocityfader", argv[i], "velocityfader")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			controller_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--inverted") == 0)
		{
			inverted = 1;
			controller_value = 0;
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
	return 0;
}


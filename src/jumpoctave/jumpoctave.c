
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int down_transposition[128];
static int transposition = 0;
static int pitch_wheel_state = 0;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			int note = MidiUtilNoteOffMessage_getNote(message);
			int new_note = note + down_transposition[note];

			if (new_note >= 0 && new_note < 128)
			{
				unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
				MidiUtilMessage_setNoteOff(new_message, MidiUtilNoteOffMessage_getChannel(message), new_note, MidiUtilNoteOffMessage_getVelocity(message));
				rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int note = MidiUtilNoteOnMessage_getNote(message);
			int new_note = note + transposition;
			down_transposition[note] = transposition;

			if (new_note >= 0 && new_note < 128)
			{
				unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
				MidiUtilMessage_setNoteOn(new_message, MidiUtilNoteOnMessage_getChannel(message), new_note, MidiUtilNoteOnMessage_getVelocity(message));
				rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL:
		{
			int value = MidiUtilPitchWheelMessage_getValue(message);

			if (value > 0x3500)
			{
				if (pitch_wheel_state != 1)
				{
					pitch_wheel_state = 1;
					transposition += 12;
				}
			}
			else if (value < 0x500)
			{
				if (pitch_wheel_state != -1)
				{
					pitch_wheel_state = -1;
					transposition -= 12;
				}
			}
			else
			{
				pitch_wheel_state = 0;
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

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
}

int main(int argc, char **argv)
{
	int i;

	for (i = 0; i < 128; i++) down_transposition[i] = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("jumpoctave", argv[i], "jumpoctave", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("jumpoctave", argv[i], "jumpoctave")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL)) usage(argv[0]);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


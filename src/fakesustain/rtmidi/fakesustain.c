
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil.h>

typedef enum
{
	NOTE_STATE_OFF,
	NOTE_STATE_ON,
	NOTE_STATE_SUSTAINED
}
NoteState_t;

static int sustain[16];
static NoteState_t note_state[16][128];
static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;

static void send_note_on(int channel, int note, int velocity)
{
	unsigned char message[RTMIDI_MESSAGE_SIZE_NOTE_ON];
	rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_NOTE_ON);
	rtmidi_note_on_message_set_channel(message, channel);
	rtmidi_note_on_message_set_note(message, note);
	rtmidi_note_on_message_set_velocity(message, velocity);
	rtmidi_out_send_message(midi_out, message, RTMIDI_MESSAGE_SIZE_NOTE_ON);
}

static void send_note_off(int channel, int note)
{
	unsigned char message[RTMIDI_MESSAGE_SIZE_NOTE_OFF];
	rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_NOTE_OFF);
	rtmidi_note_off_message_set_channel(message, channel);
	rtmidi_note_off_message_set_note(message, note);
	rtmidi_note_off_message_set_velocity(message, 0);
	rtmidi_out_send_message(midi_out, message, RTMIDI_MESSAGE_SIZE_NOTE_OFF);
}

static void handle_note_on(int channel, int note, int velocity)
{
	switch (note_state[channel][note])
	{
		case NOTE_STATE_ON:
		{
			send_note_off(channel, note);
			send_note_on(channel, note, velocity);
			break;
		}
		case NOTE_STATE_OFF:
		{
			send_note_on(channel, note, velocity);
			note_state[channel][note] = NOTE_STATE_ON;
			break;
		}
		case NOTE_STATE_SUSTAINED:
		{
			send_note_off(channel, note);
			send_note_on(channel, note, velocity);
			note_state[channel][note] = NOTE_STATE_ON;
			break;
		}
	}
}

static void handle_note_off(int channel, int note)
{
	if (note_state[channel][note] == NOTE_STATE_ON)
	{
		if (sustain[channel])
		{
			note_state[channel][note] = NOTE_STATE_SUSTAINED;
		}
		else
		{
			send_note_off(channel, note);
			note_state[channel][note] = NOTE_STATE_OFF;
		}
	}
}

static void handle_sustain_on(int channel)
{
	sustain[channel] = 1;
}

static void handle_sustain_off(int channel)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_state[channel][note] == NOTE_STATE_SUSTAINED)
		{
			send_note_off(channel, note);
			note_state[channel][note] = NOTE_STATE_OFF;
		}
	}

	sustain[channel] = 0;
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (rtmidi_message_get_type(message))
	{
		case RTMIDI_MESSAGE_TYPE_NOTE_OFF:
		{
			handle_note_off(rtmidi_note_off_message_get_channel(message), rtmidi_note_off_message_get_note(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_NOTE_ON:
		{
			if (rtmidi_note_on_message_get_velocity(message) == 0)
			{
				handle_note_off(rtmidi_note_on_message_get_channel(message), rtmidi_note_on_message_get_note(message));
			}
			else
			{
				handle_note_on(rtmidi_note_on_message_get_channel(message), rtmidi_note_on_message_get_note(message), rtmidi_note_on_message_get_velocity(message));
			}

			break;
		}
		case RTMIDI_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			if (rtmidi_control_change_message_get_number(message) == 64)
			{
				if (rtmidi_control_change_message_get_value(message) >= 64)
				{
					handle_sustain_on(rtmidi_control_change_message_get_channel(message));
				}
				else
				{
					handle_sustain_off(rtmidi_control_change_message_get_channel(message));
				}
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
	fprintf(stderr, "Usage:  %s --in <port> --out <port>\n", program_name);
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

			if ((midi_in = rtmidi_open_in_port("fakesustain", argv[i], "in", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("fakesustain", argv[i], "out")) == NULL)
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

	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	return 0;
}


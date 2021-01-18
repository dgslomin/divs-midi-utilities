
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

/* 
 * Note:  This uses an embedded copy of the old fakesustain utility's logic
 * so that the pulsar logic doesn't need to handle sustain directly.
 */

typedef enum
{
	NOTE_STATE_OFF,
	NOTE_STATE_ON,
	NOTE_STATE_SUSTAINED
}
NoteState_t;

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int number_of_pulses = 0;
static int pulse_on_msecs_array[128];
static int pulse_off_msecs_array[128];
static int should_shutdown = 0;
static int sustain[16];
static NoteState_t note_state[16][128];
static int pulse_is_on = 0;
static int sustained_note_velocity[16][128];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --pulse <on msecs> <off msecs> ] ...\n", program_name);
	exit(1);
}

static void send_note_on(int channel, int note, int velocity)
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
	MidiUtilMessage_setNoteOn(message, channel, note, velocity);
	rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
}

static void send_note_off(int channel, int note)
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
	MidiUtilMessage_setNoteOff(message, channel, note, 0);
	rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
}

static void handle_interrupt(void *user_data)
{
	should_shutdown = 1;
}

static void handle_sustained_note_on(int channel, int note, int velocity)
{
	if (pulse_is_on)
	{
		if (sustained_note_velocity[channel][note] > 0) send_note_off(channel, note);
		send_note_on(channel, note, velocity);
	}

	sustained_note_velocity[channel][note] = velocity;
}

static void handle_sustained_note_off(int channel, int note)
{
	if (pulse_is_on && (sustained_note_velocity[channel][note] > 0)) send_note_off(channel, note);
	sustained_note_velocity[channel][note] = 0;
}

static void handle_pulse_on(void)
{
	int channel, note;

	for (channel = 0; channel < 16; channel++)
	{
		for (note = 0; note < 128; note++)
		{
			if (sustained_note_velocity[channel][note] > 0) send_note_on(channel, note, sustained_note_velocity[channel][note]);
		}
	}

	pulse_is_on = 1;
}

static void handle_pulse_off(void)
{
	int channel, note;

	for (channel = 0; channel < 16; channel++)
	{
		for (note = 0; note < 128; note++)
		{
			if (sustained_note_velocity[channel][note] > 0) send_note_off(channel, note);
		}
	}

	pulse_is_on = 0;
}

static void handle_note_on(int channel, int note, int velocity)
{
	switch (note_state[channel][note])
	{
		case NOTE_STATE_ON:
		{
			handle_sustained_note_off(channel, note);
			handle_sustained_note_on(channel, note, velocity);
			break;
		}
		case NOTE_STATE_OFF:
		{
			handle_sustained_note_on(channel, note, velocity);
			note_state[channel][note] = NOTE_STATE_ON;
			break;
		}
		case NOTE_STATE_SUSTAINED:
		{
			handle_sustained_note_off(channel, note);
			handle_sustained_note_on(channel, note, velocity);
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
			handle_sustained_note_off(channel, note);
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
			handle_sustained_note_off(channel, note);
			note_state[channel][note] = NOTE_STATE_OFF;
		}
	}

	sustain[channel] = 0;
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			handle_note_off(MidiUtilNoteOffMessage_getChannel(message), MidiUtilNoteOffMessage_getNote(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int velocity = MidiUtilNoteOnMessage_getVelocity(message);

			if (velocity == 0)
			{
				handle_note_off(MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message));
			}
			else
			{
				handle_note_on(MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message), velocity);
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			if (MidiUtilControlChangeMessage_getNumber(message) == 64)
			{
				int channel = MidiUtilControlChangeMessage_getChannel(message);

				if (MidiUtilControlChangeMessage_getValue(message) >= 64)
				{
					handle_sustain_on(channel);
				}
				else
				{
					handle_sustain_off(channel);
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

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("pulsar", argv[i], "pulsar", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("pulsar", argv[i], "pulsar")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--pulse") == 0)
		{
			if (++i == argc) usage(argv[0]);
			pulse_on_msecs_array[number_of_pulses] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			pulse_off_msecs_array[number_of_pulses] = atoi(argv[i]);
			number_of_pulses++;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL)) usage(argv[0]);
	MidiUtil_setInterruptHandler(handle_interrupt, NULL);

	while (!should_shutdown)
	{
		int pulse_number;

		for (pulse_number = 0; pulse_number < number_of_pulses; pulse_number++)
		{
			handle_pulse_on();
			MidiUtil_sleep(pulse_on_msecs_array[pulse_number]);
			if (should_shutdown) break;

			handle_pulse_off();
			MidiUtil_sleep(pulse_off_msecs_array[pulse_number]);
			if (should_shutdown) break;
		}
	}

	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int do_sustain = 0;
static int do_sustenuto = 0;
static int do_bass_sustain = 0;
static int do_soft = 0;
static int do_volume = 0;
static int independent_sustenuto = 0;
static int highest_bass_note = 59;
static int max_soft_velocity = 95;
static int sustain_controller_number = 64;
static int sustenuto_controller_number = 66;
static int bass_sustain_controller_number = 69;
static int soft_controller_number = 67;
static int volume_controller_number = 12;
static int invert_sustain = 0;
static int invert_sustenuto = 0;
static int invert_bass_sustain = 0;
static int invert_soft = 0;
static int invert_volume = 0;
static int sustain_down[16];
static int bass_sustain_down[16];
static int soft_down[16];
static int volume[16];
static int note_down[16][128];
static int note_held_by_sustain[16][128];
static int note_included_in_sustenuto[16][128];
static int note_held_by_sustenuto[16][128];
static int note_held_by_bass_sustain[16][128];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --sustain ] [ --sustenuto ] [ --bass-sustain ] [ --soft ] [ --volume ] [ --independent-sustenuto ] [ --highest-bass-note <default B3> ] [ --max-soft-velocity <default 95> ] [ --sustain-controller <default 64> ] [ --sustenuto-controller <default 66> ] [ --bass-sustain-controller <default 69> ] [ --soft-controller <default 67> ] [ --volume-controller <default 12> ] [ --invert-sustain ] [ --invert-sustenuto ] [ --invert-bass-sustain ] [ --invert-soft ] [ --invert-volume ]\n", program_name);
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

static void handle_note_on(int channel, int note, int velocity)
{
	int scaled_velocity = velocity * (soft_down[channel] ? max_soft_velocity : 127) * volume[channel] / 127 / 127;
	if (note_down[channel][note] || note_held_by_sustain[channel][note] || note_held_by_sustenuto[channel][note] || note_held_by_bass_sustain[channel][note]) send_note_off(channel, note);
	send_note_on(channel, note, scaled_velocity);
	note_down[channel][note] = 1;
	note_held_by_sustain[channel][note] = 0;
	note_held_by_sustenuto[channel][note] = 0;
	note_held_by_bass_sustain[channel][note] = 0;
}

static void handle_note_off(int channel, int note)
{
	if (note_down[channel][note])
	{
		if (sustain_down[channel]) note_held_by_sustain[channel][note] = 1;
		if (note_included_in_sustenuto[channel][note]) note_held_by_sustenuto[channel][note] = 1;
		if (bass_sustain_down[channel] && (note <= highest_bass_note)) note_held_by_bass_sustain[channel][note] = 1;
		if (!(note_held_by_sustain[channel][note] || note_held_by_sustenuto[channel][note] || note_held_by_bass_sustain[channel][note])) send_note_off(channel, note);
		note_down[channel][note] = 0;
	}
}

static void handle_sustain_on(int channel)
{
	sustain_down[channel] = 1;
}

static void handle_sustain_off(int channel)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_held_by_sustain[channel][note])
		{
			if (!(note_held_by_sustenuto[channel][note] || note_held_by_bass_sustain[channel][note])) send_note_off(channel, note);
			note_held_by_sustain[channel][note] = 0;
		}
	}

	sustain_down[channel] = 0;
}

static void handle_sustenuto_on(int channel)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_down[channel][note] || (!independent_sustenuto && (sustain_down[channel] || (bass_sustain_down[channel] && (note <= highest_bass_note))))) note_included_in_sustenuto[channel][note] = 1;
	}
}

static void handle_sustenuto_off(int channel)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_held_by_sustenuto[channel][note])
		{
			if (!(note_held_by_sustain[channel][note] || note_held_by_bass_sustain[channel][note])) send_note_off(channel, note);
			note_held_by_sustenuto[channel][note] = 0;
		}

		note_included_in_sustenuto[channel][note] = 0;
	}
}

static void handle_bass_sustain_on(int channel)
{
	bass_sustain_down[channel] = 1;
}

static void handle_bass_sustain_off(int channel)
{
	int note;

	for (note = 0; note <= highest_bass_note; note++)
	{
		if (note_held_by_bass_sustain[channel][note])
		{
			if (!(note_held_by_sustain[channel][note] || note_held_by_sustenuto[channel][note])) send_note_off(channel, note);
			note_held_by_bass_sustain[channel][note] = 0;
		}
	}

	bass_sustain_down[channel] = 0;
}

static void handle_soft_on(int channel)
{
	soft_down[channel] = 1;
}

static void handle_soft_off(int channel)
{
	soft_down[channel] = 0;
}

static void handle_volume(int channel, int value)
{
	volume[channel] = value;
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
			int channel = MidiUtilControlChangeMessage_getChannel(message);
			int number = MidiUtilControlChangeMessage_getNumber(message);
			int value = MidiUtilControlChangeMessage_getValue(message);
			int value_above_middle = (value >= 64);

			if (do_sustain && (number == sustain_controller_number))
			{
				if (!value_above_middle == !invert_sustain)
				{
					handle_sustain_off(channel);
				}
				else
				{
					handle_sustain_on(channel);
				}
			}
			else if (do_sustenuto && (number == sustenuto_controller_number))
			{
				if (!value_above_middle == !invert_sustenuto)
				{
					handle_sustenuto_off(channel);
				}
				else
				{
					handle_sustenuto_on(channel);
				}
			}
			else if (do_bass_sustain && (number == bass_sustain_controller_number))
			{
				if (!value_above_middle == !invert_bass_sustain)
				{
					handle_bass_sustain_off(channel);
				}
				else
				{
					handle_bass_sustain_on(channel); 
				}
			}
			else if (do_soft && (number == soft_controller_number))
			{
				if (!value_above_middle == !invert_soft)
				{
					handle_soft_off(channel);
				}
				else
				{
					handle_soft_on(channel);
				}
			}
			else if (do_volume && (number == volume_controller_number))
			{
				handle_volume(channel, invert_volume ? 127 - value : value);
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

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
}

int main(int argc, char **argv)
{
	int i, j;

	for (i = 0; i < 16; i++)
	{
		sustain_down[i] = 0;
		bass_sustain_down[i] = 0;
		soft_down[i] = 0;
		volume[i] = 127;

		for (j = 0; j < 128; j++)
		{
			note_down[i][j] = 0;
			note_held_by_sustain[i][j] = 0;
			note_included_in_sustenuto[i][j] = 0;
			note_held_by_sustenuto[i][j] = 0;
			note_held_by_bass_sustain[i][j] = 0;
		}
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("pedalsim", argv[i], "pedalsim", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("pedalsim", argv[i], "pedalsim")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--sustain") == 0)
		{
			do_sustain = 1;
		}
		else if (strcmp(argv[i], "--sustenuto") == 0)
		{
			do_sustenuto = 1;
		}
		else if (strcmp(argv[i], "--bass-sustain") == 0)
		{
			do_bass_sustain = 1;
		}
		else if (strcmp(argv[i], "--soft") == 0)
		{
			do_soft = 1;
		}
		else if (strcmp(argv[i], "--volume") == 0)
		{
			do_volume = 1;
		}
		else if (strcmp(argv[i], "--independent-sustenuto") == 0)
		{
			independent_sustenuto = 1;
		}
		else if (strcmp(argv[i], "--highest-bass-note") == 0)
		{
			if (++i == argc) usage(argv[0]);
			highest_bass_note = MidiUtil_getNoteNumberFromName(argv[i]);
		}
		else if (strcmp(argv[i], "--max-soft-velocity") == 0)
		{
			if (++i == argc) usage(argv[0]);
			max_soft_velocity = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--sustain-controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sustain_controller_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--sustenuto-controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sustenuto_controller_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--bass-sustain-controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			bass_sustain_controller_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--soft-controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			soft_controller_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--volume-controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			volume_controller_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--invert-sustain") == 0)
		{
			invert_sustain = 1;
		}
		else if (strcmp(argv[i], "--invert-sustenuto") == 0)
		{
			invert_sustenuto = 1;
		}
		else if (strcmp(argv[i], "--invert-bass-sustain") == 0)
		{
			invert_bass_sustain = 1;
		}
		else if (strcmp(argv[i], "--invert-soft") == 0)
		{
			invert_soft = 1;
		}
		else if (strcmp(argv[i], "--invert-volume") == 0)
		{
			invert_volume = 1;
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


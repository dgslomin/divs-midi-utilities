
/*
 * This implementation treats the provided pattern as a MIDI sequence which
 * modulates the notes that are played interactively.  Players for the
 * sequence, each with its own notion of the "current event", are continuously
 * created and destroyed in response to the notes being played interactively.
 * Due to the large number of simultaneous players, it uses a game loop rather
 * than alarms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>
#include "noteflurry.h"
#include "echo.h"
#include "seq.h"
#include "multiseq.h"

typedef enum
{
	MODE_INVALID = -1,
	MODE_ECHO,
	MODE_SEQ,
	MODE_MULTISEQ
}
Mode_t;

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static MidiUtilLock_t lock;
static Mode_t mode = MODE_INVALID;

MidiFile_t midi_file;
float tempo_bpm = 100;
int note_velocity[16][128];
int note_sustain[16][128];
int channel_sustain[16];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> ( --echo | --seq | --multiseq | --arp | --marimba ) [ --note <beat> <duration beats> <note interval> <velocity> ] ... [ --loop <beats> ] [ --direction ( up | down | up-down | down-up | rolling-up-down | rolling-down-up | random ) ] [ --octaves <default 1> ] [ --duty-cycle <default 0.5> ] [ --tempo <bpm, default 100> ]\n", program_name);
	exit(1);
}

void send_note_on(int channel, int note, int velocity)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
		MidiUtilMessage_setNoteOn(message, channel, note, velocity);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
	}
}

void send_note_off(int channel, int note, int velocity)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
		MidiUtilMessage_setNoteOff(message, channel, note, velocity);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
	}
}

static void handle_virtual_note_on(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	switch (mode)
	{
		case MODE_ECHO:
		{
			echo_note_on(current_time_msecs, channel, note, velocity);
			break;
		}
		case MODE_SEQ:
		{
			seq_note_on(current_time_msecs, channel, note, velocity);
			break;
		}
		case MODE_MULTISEQ:
		{
			multiseq_note_on(current_time_msecs, channel, note, velocity);
			break;
		}
		default:
		{
			break;
		}
	}
}

static void handle_virtual_note_off(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	switch (mode)
	{
		case MODE_ECHO:
		{
			echo_note_off(current_time_msecs, channel, note, velocity);
			break;
		}
		case MODE_SEQ:
		{
			seq_note_off(current_time_msecs, channel, note, velocity);
			break;
		}
		case MODE_MULTISEQ:
		{
			multiseq_note_off(current_time_msecs, channel, note, velocity);
			break;
		}
		default:
		{
			break;
		}
	}
}

static void handle_note_on(int channel, int note, int velocity)
{
	if (note_velocity[channel][note] > 0) handle_virtual_note_off(channel, note, 0);
	handle_virtual_note_on(channel, note, velocity);
	note_velocity[channel][note] = velocity;
	note_sustain[channel][note] = 0;
}

static void handle_note_off(int channel, int note, int velocity)
{
	if ((note_velocity[channel][note] > 0) && !note_sustain[channel][note])
	{
		if (channel_sustain[channel])
		{
			note_sustain[channel][note] = 1;
		}
		else
		{
			handle_virtual_note_off(channel, note, velocity);
			note_velocity[channel][note] = 0;
		}
	}
}

static void handle_sustain_on(int channel)
{
	channel_sustain[channel] = 1;
}

static void handle_sustain_off(int channel)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_sustain[channel][note])
		{
			handle_virtual_note_off(channel, note, 0);
			note_velocity[channel][note] = 0;
			note_sustain[channel][note] = 0;
		}
	}

	channel_sustain[channel] = 0;
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	MidiUtilLock_lock(lock);

	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			handle_note_off(MidiUtilNoteOffMessage_getChannel(message), MidiUtilNoteOffMessage_getNote(message), MidiUtilNoteOffMessage_getVelocity(message));
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int velocity = MidiUtilNoteOnMessage_getVelocity(message);

			if (velocity == 0)
			{
				handle_note_off(MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message), 0);
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

	MidiUtilLock_unlock(lock);
}

static void player_thread_main(void *user_data)
{
	long current_time_msecs;

	while (1)
	{
		MidiUtilLock_lock(lock);
		current_time_msecs = MidiUtil_getCurrentTimeMsecs();

		switch (mode)
		{
			case MODE_ECHO:
			{
				echo_update_players(current_time_msecs);
				break;
			}
			case MODE_SEQ:
			{
				seq_update_players(current_time_msecs);
				break;
			}
			case MODE_MULTISEQ:
			{
				multiseq_update_players(current_time_msecs);
				break;
			}
			default:
			{
				break;
			}
		}

		MidiUtilLock_unlock(lock);
		MidiUtil_sleep(1); /* tune this for CPU busyness vs responsiveness */
	}
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);

	switch (mode)
	{
		case MODE_ECHO:
		{
			echo_cleanup();
			break;
		}
		case MODE_SEQ:
		{
			seq_cleanup();
			break;
		}
		case MODE_MULTISEQ:
		{
			multiseq_cleanup();
			break;
		}
		default:
		{
			break;
		}
	}

	MidiUtilLock_free(lock);
	MidiFile_free(midi_file);
}

int main(int argc, char **argv)
{
	int i, j;

	midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	lock = MidiUtilLock_new();

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 128; j++)
		{
			note_velocity[i][j] = 0;
			note_sustain[i][j] = 0;
		}

		channel_sustain[i] = 0;
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("noteflurry", argv[i], "noteflurry", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("noteflurry", argv[i], "noteflurry")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--echo") == 0)
		{
			mode = MODE_ECHO;
		}
		else if (strcmp(argv[i], "--seq") == 0)
		{
			mode = MODE_SEQ;
		}
		else if (strcmp(argv[i], "--multiseq") == 0)
		{
			mode = MODE_MULTISEQ;
		}
		else if (strcmp(argv[i], "--note") == 0)
		{
			float beat;
			float duration_beats;
			int note_interval;
			int velocity;
			if (++i == argc) usage(argv[0]);
			beat = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			duration_beats = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_interval = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			velocity = atof(argv[i]);
			MidiFileTrack_createNoteStartAndEndEvents(MidiFile_getTrackByNumber(midi_file, 1, 1), MidiFile_getTickFromBeat(midi_file, beat), MidiFile_getTickFromBeat(midi_file, beat + duration_beats), 0, 60 + note_interval, velocity, 0);
		}
		else if (strcmp(argv[i], "--loop") == 0)
		{
			float beats;
			if (++i == argc) usage(argv[0]);
			beats = atof(argv[i]);
			MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(midi_file, 0, 1), MidiFile_getTickFromBeat(midi_file, beats), "loop");
		}
		else if (strcmp(argv[i], "--tempo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			tempo_bpm = atof(argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL)) usage(argv[0]);

	switch (mode)
	{
		case MODE_ECHO:
		{
			echo_init();
			break;
		}
		case MODE_SEQ:
		{
			seq_init();
			break;
		}
		case MODE_MULTISEQ:
		{
			multiseq_init();
			break;
		}
		default:
		{
			usage(argv[0]);
			break;
		}
	}

	MidiUtil_startThread(player_thread_main, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


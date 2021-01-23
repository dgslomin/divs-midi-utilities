
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static MidiUtilAlarm_t alarm;
static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static float tempo_bpm = 100;
static int tempo_controller_number = -1;
static float tempo_controller_low_scale = 1.0;
static float tempo_controller_high_scale = 1.0;
static int number_of_echoes = 0;
static float echo_beat_array[128];
static int echo_note_interval_array[128];
static float echo_velocity_scale_array[128];
static int number_of_arps = 0;
static float arp_on_beat_array[128];
static float arp_off_beat_array[128];
static int arp_note_interval_array[128];
static float arp_velocity_scale_array[128];
static int number_of_seqs = 0;
static float seq_on_beat_array[128];
static float seq_off_beat_array[128];
static int seq_note_interval_array[128];
static float seq_velocity_scale_array[128];
static float arp_length_beats = -1.0;
static float seq_length_beats = 4.0;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --tempo <bpm, default 100> ] [ --tempo-controller <number> <low scale> <high scale> ] [ --echo <beat> <note interval> <velocity scale> | --arp <on beat> <off beat> <note interval> <velocity scale> | --seq <on beat> <off beat> <note interval> <velocity scale> ] ... [ --arp-length <beats> ] [ --seq-length <beats, default 4> ]\n", program_name);
	exit(1);
}

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
				int new_velocity = MidiUtil_clamp((int)(MidiUtilNoteOffMessage_getVelocity(message) * echo_velocity_scale_array[echo_number]), 0, 127);

				if (new_note >= 0 && new_note < 128)
				{
					unsigned char *new_message = (unsigned char *)(malloc(MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF));
					MidiUtilMessage_setNoteOff(new_message, MidiUtilNoteOffMessage_getChannel(message), new_note, new_velocity);
					MidiUtilAlarm_add(alarm, echo_beat_array[echo_number] * 60000 / tempo_bpm, handle_alarm, new_message);
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
				int new_velocity = MidiUtil_clamp((int)(MidiUtilNoteOnMessage_getVelocity(message) * echo_velocity_scale_array[echo_number]), 0, 127);

				if (new_note >= 0 && new_note < 128)
				{
					unsigned char *new_message = (unsigned char *)(malloc(MIDI_UTIL_MESSAGE_SIZE_NOTE_ON));
					MidiUtilMessage_setNoteOn(new_message, MidiUtilNoteOnMessage_getChannel(message), new_note, new_velocity);
					MidiUtilAlarm_add(alarm, echo_beat_array[echo_number] * 60000 / tempo_bpm, handle_alarm, new_message);
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

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	MidiUtilAlarm_free(alarm);
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
		else if (strcmp(argv[i], "--tempo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			tempo_bpm = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--tempo-controller") == 0)
		{
			if (++i == argc) usage(argv[0]);
			tempo_controller_number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			tempo_controller_low_scale = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			tempo_controller_high_scale = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--echo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			echo_beat_array[number_of_echoes] = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			echo_note_interval_array[number_of_echoes] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			echo_velocity_scale_array[number_of_echoes] = atof(argv[i]);
			number_of_echoes++;
		}
		else if (strcmp(argv[i], "--arp") == 0)
		{
			if (++i == argc) usage(argv[0]);
			arp_on_beat_array[number_of_arps] = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			arp_off_beat_array[number_of_arps] = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			arp_note_interval_array[number_of_arps] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			arp_velocity_scale_array[number_of_arps] = atof(argv[i]);
			number_of_arps++;
		}
		else if (strcmp(argv[i], "--seq") == 0)
		{
			if (++i == argc) usage(argv[0]);
			seq_on_beat_array[number_of_seqs] = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			seq_off_beat_array[number_of_seqs] = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			seq_note_interval_array[number_of_seqs] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			seq_velocity_scale_array[number_of_seqs] = atof(argv[i]);
			number_of_seqs++;
		}
		else if (strcmp(argv[i], "--arp-length") == 0)
		{
			if (++i == argc) usage(argv[0]);
			arp_length_beats = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--seq-length") == 0)
		{
			if (++i == argc) usage(argv[0]);
			seq_length_beats = atof(argv[i]);
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


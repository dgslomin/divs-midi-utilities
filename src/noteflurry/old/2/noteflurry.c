
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

struct LiveNote
{
	long on_time_msecs;
	long off_time_msecs;
	int channel;
	int note;
	int on_velocity;
	int off_velocity;
	int on_echo_number;
	int off_echo_number;
};

typedef struct LiveNote *LiveNote_t;

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
static MidiUtilLock_t lock;
static MidiUtilPointerArray_t live_note_array;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --tempo <bpm, default 100> ] [ --tempo-controller <number> <low scale> <high scale> ] [ --echo <beat> <note interval> <velocity scale> | --arp <on beat> <off beat> <note interval> <velocity scale> | --seq <on beat> <off beat> <note interval> <velocity scale> ] ... [ --arp-length <beats> ] [ --seq-length <beats, default 4> ]\n", program_name);
	exit(1);
}

static void send_note_on(int channel, int note, int velocity)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
		MidiUtilMessage_setNoteOn(message, channel, note, velocity);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
	}
}

static void send_note_off(int channel, int note, int velocity)
{
	if (midi_out != NULL)
	{
		unsigned char message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
		MidiUtilMessage_setNoteOff(message, channel, note, velocity);
		rtmidi_out_send_message(midi_out, message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
	}
}

static void handle_note_on(int channel, int note, int velocity)
{
	LiveNote_t live_note = (LiveNote_t)(malloc(sizeof(struct LiveNote)));
	live_note->on_time_msecs = MidiUtil_getCurrentTimeMsecs();
	live_note->off_time_msecs = live_note->on_time_msecs; /* will fill in a meaningful value at note off */
	live_note->channel = channel;
	live_note->note = note;
	live_note->on_velocity = velocity;
	live_note->off_velocity = -1; /* can check this to see if note off happened */
	live_note->on_echo_number = 0;
	live_note->off_echo_number = 0;

	MidiUtilLock_lock(lock);
	MidiUtilPointerArray_add(live_note_array, live_note);
	MidiUtilLock_unlock(lock);
}

static void handle_note_off(int channel, int note, int velocity)
{
	int number_of_live_notes, live_note_number;

	MidiUtilLock_lock(lock);
	number_of_live_notes = MidiUtilPointerArray_getSize(live_note_array);

	for (live_note_number = 0; live_note_number < number_of_live_notes; live_note_number++)
	{
		LiveNote_t live_note = (LiveNote_t)(MidiUtilPointerArray_get(live_note_array, live_note_number));

		/* turn off the oldest matching note that's still on */
		if ((live_note->channel == channel) && (live_note->note == note) && (live_note->off_velocity < 0))
		{
			live_note->off_time_msecs = MidiUtil_getCurrentTimeMsecs();
			live_note->off_velocity = velocity;
			break;
		}
	}

	MidiUtilLock_unlock(lock);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			int channel = MidiUtilNoteOffMessage_getChannel(message);
			int note = MidiUtilNoteOffMessage_getNote(message);
			int velocity = MidiUtilNoteOffMessage_getVelocity(message);
			handle_note_off(channel, note, velocity);
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int channel = MidiUtilNoteOnMessage_getChannel(message);
			int note = MidiUtilNoteOnMessage_getNote(message);
			int velocity = MidiUtilNoteOnMessage_getVelocity(message);

			if (velocity == 0)
			{
				handle_note_off(channel, note, 0);
			}
			else
			{
				handle_note_on(channel, note, velocity);
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

static void loop_thread_main(void *user_data)
{
	while (1)
	{
		float current_beat;
		int live_note_number;

		MidiUtilLock_lock(lock);
		current_beat = MidiUtil_getCurrentTimeMsecs() * tempo_bpm / 60000;

		for (live_note_number = 0; live_note_number < MidiUtilPointerArray_getSize(live_note_array); live_note_number++)
		{
			LiveNote_t live_note = (LiveNote_t)(MidiUtilPointerArray_get(live_note_array, live_note_number));
			float on_beat = live_note->on_time_msecs * tempo_bpm / 60000;

			if (live_note->on_echo_number < number_of_echoes)
			{
				float on_echo_beat = on_beat + echo_beat_array[live_note->on_echo_number];

				if (current_beat >= on_echo_beat)
				{
					int new_note = live_note->note + echo_note_interval_array[live_note->on_echo_number];

					if (new_note >= 0 && new_note < 128)
					{
						int new_velocity = (int)(live_note->on_velocity * echo_velocity_scale_array[live_note->on_echo_number]);
						send_note_on(live_note->channel, new_note, new_velocity);
					}

					(live_note->on_echo_number)++;
				}
			}

			if ((live_note->off_velocity >= 0) && (live_note->off_echo_number < number_of_echoes))
			{
				long off_echo_beat = on_beat + echo_beat_array[live_note->off_echo_number];

				if (current_beat >= off_echo_beat)
				{
					int new_note = live_note->note + echo_note_interval_array[live_note->off_echo_number];

					if (new_note >= 0 && new_note < 128)
					{
						int new_velocity = (int)(live_note->off_velocity * echo_velocity_scale_array[live_note->off_echo_number]);
						send_note_off(live_note->channel, new_note, new_velocity);
					}

					(live_note->off_echo_number)++;
				}
			}

			// TODO: there's a logic problem when echo_beat_array isn't in increasing order, which you need to fix before doing arps, since it's exacerbated there
			// maybe sort echo_beat_array, arp_on_beat_array, arp_off_beat_array, and have separate "current" counters for arp_on and arp_off?
			// now it's looking like treating the echo, arp, and seq as three MidiFile_t objects may be better

#if 0
			if (live_note->arp_number < number_of_arps)
			{
				if (live_note->off_velocity < 0)
				{
					for (arp_number = 0; arp_number < number_of_arps; arp_number++)
					{
						long arp_on_beat = on_beat + arp_on_beat_array[arp_number];
						long arp_off_beat = on_beat + arp_off_beat_array[arp_number];

						if (current_beat >= arp_on_beat)
						{
						}
					}
				}
			}
#endif

			/* no longer live if there's nothing left for it to do */
			if ((live_note->on_echo_number == number_of_echoes) && (live_note->off_echo_number == number_of_echoes))
			{
				MidiUtilPointerArray_remove(live_note_array, live_note_number--);
				free(live_note);
			}
		}

		MidiUtilLock_unlock(lock);
		MidiUtil_sleep(5); /* tune this for CPU busyness vs responsiveness */
	}
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	MidiUtilPointerArray_free(live_note_array);
	MidiUtilLock_free(lock);
}

int main(int argc, char **argv)
{
	int i;
	lock = MidiUtilLock_new();
	live_note_array = MidiUtilPointerArray_new(1024);

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
	MidiUtil_startThread(loop_thread_main, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


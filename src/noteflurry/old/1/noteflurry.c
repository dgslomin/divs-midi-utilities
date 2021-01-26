
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

struct ScheduledMessage
{
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_SHORT_MESSAGE];
	int original_note;
	void *original_note_id;
};

typedef struct NoteHoldId *NoteHoldId_t;
typedef struct ScheduledMessage *ScheduledMessage_t;

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
static void *original_note_id_array[128];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --tempo <bpm, default 100> ] [ --tempo-controller <number> <low scale> <high scale> ] [ --echo <beat> <note interval> <velocity scale> | --arp <on beat> <off beat> <note interval> <velocity scale> | --seq <on beat> <off beat> <note interval> <velocity scale> ] ... [ --arp-length <beats> ] [ --seq-length <beats, default 4> ]\n", program_name);
	exit(1);
}

static void handle_echo_alarm(int cancelled, void *user_data)
{
	ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(user_data);
	if (!cancelled) rtmidi_out_send_message(midi_out, scheduled_message->message, MidiUtilMessage_getSize(scheduled_message->message));
	free(scheduled_message);
}

static void handle_arp_on_alarm(int cancelled, void *user_data)
{
	ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(user_data);

	if (!cancelled)
	{
		if (scheduled_message->original_note_id == original_note_id_array[scheduled_message->original_note])
		{
			rtmidi_out_send_message(midi_out, scheduled_message->message, MidiUtilMessage_getSize(scheduled_message->message));
		}
	}

	free(scheduled_message);
}

static void handle_arp_off_alarm(int cancelled, void *user_data)
{
	ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(user_data);
	if (!cancelled) rtmidi_out_send_message(midi_out, scheduled_message->message, MidiUtilMessage_getSize(scheduled_message->message));
	free(scheduled_message);
}

static void handle_arp_loop_alarm(int cancelled, void *user_data)
{
	ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(user_data);

	if (!cancelled)
	{
		if (scheduled_message->original_note_id == original_note_id_array[scheduled_message->original_note])
		{
			int channel = MidiUtilNoteOnMessage_getChannel(scheduled_message->message);
			int note = MidiUtilNoteOnMessage_getNote(scheduled_message->message);
			int velocity = MidiUtilNoteOnMessage_getVelocity(scheduled_message->message);
			int i;

			for (i = 0; i < number_of_arps; i++)
			{
				int new_note = note + arp_note_interval_array[i];
				int new_velocity = MidiUtil_clamp((int)(velocity * arp_velocity_scale_array[i]), 0, 127);

				if (new_note >= 0 && new_note < 128)
				{
					{
						ScheduledMessage_t new_scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
						MidiUtilMessage_setNoteOn(new_scheduled_message->message, channel, new_note, new_velocity);
						new_scheduled_message->original_note = scheduled_message->original_note;
						new_scheduled_message->original_note_id = scheduled_message->original_note_id;
						MidiUtilAlarm_add(alarm, arp_on_beat_array[i] * 60000 / tempo_bpm, handle_arp_on_alarm, new_scheduled_message);
					}

					{
						ScheduledMessage_t new_scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
						MidiUtilMessage_setNoteOff(new_scheduled_message->message, channel, new_note, 0);
						new_scheduled_message->original_note = scheduled_message->original_note;
						new_scheduled_message->original_note_id = scheduled_message->original_note_id;
						MidiUtilAlarm_add(alarm, arp_off_beat_array[i] * 60000 / tempo_bpm, handle_arp_off_alarm, new_scheduled_message);
					}
				}
			}

			{
				ScheduledMessage_t new_scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
				MidiUtilMessage_setNoteOn(new_scheduled_message->message, channel, note, velocity);
				new_scheduled_message->original_note = scheduled_message->original_note;
				new_scheduled_message->original_note_id = scheduled_message->original_note_id;
				MidiUtilAlarm_add(alarm, arp_length_beats * 60000 / tempo_bpm, handle_arp_loop_alarm, new_scheduled_message);
			}
		}
	}

	free(scheduled_message);
}

static void handle_note_on(int channel, int note, int velocity)
{
	void *original_note_id;
	int i;

	original_note_id = malloc(1);
	original_note_id_array[note] = original_note_id;

	for (i = 0; i < number_of_echoes; i++)
	{
		int new_note = note + echo_note_interval_array[i];
		int new_velocity = MidiUtil_clamp((int)(velocity * echo_velocity_scale_array[i]), 0, 127);

		if (new_note >= 0 && new_note < 128)
		{
			ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
			MidiUtilMessage_setNoteOn(scheduled_message->message, channel, new_note, new_velocity);
			MidiUtilAlarm_add(alarm, echo_beat_array[i] * 60000 / tempo_bpm, handle_echo_alarm, scheduled_message);
		}
	}

	for (i = 0; i < number_of_arps; i++)
	{
		int new_note = note + arp_note_interval_array[i];
		int new_velocity = MidiUtil_clamp((int)(velocity * arp_velocity_scale_array[i]), 0, 127);

		if (new_note >= 0 && new_note < 128)
		{
			{
				ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
				MidiUtilMessage_setNoteOn(scheduled_message->message, channel, new_note, new_velocity);
				scheduled_message->original_note = note;
				scheduled_message->original_note_id = original_note_id;
				MidiUtilAlarm_add(alarm, arp_on_beat_array[i] * 60000 / tempo_bpm, handle_arp_on_alarm, scheduled_message);
			}

			{
				ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
				MidiUtilMessage_setNoteOff(scheduled_message->message, channel, new_note, 0);
				scheduled_message->original_note = note;
				scheduled_message->original_note_id = original_note_id;
				MidiUtilAlarm_add(alarm, arp_off_beat_array[i] * 60000 / tempo_bpm, handle_arp_off_alarm, scheduled_message);
			}
		}
	}

	if (arp_length_beats > 0)
	{
		ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
		MidiUtilMessage_setNoteOn(scheduled_message->message, channel, note, velocity);
		scheduled_message->original_note = note;
		scheduled_message->original_note_id = original_note_id;
		MidiUtilAlarm_add(alarm, arp_length_beats * 60000 / tempo_bpm, handle_arp_loop_alarm, scheduled_message);
	}
}

static void handle_note_off(int channel, int note, int velocity)
{
 	void *original_note_id;
	int i;

	original_note_id = original_note_id_array[note];

	if (original_note_id != NULL)
	{
		original_note_id_array[note] = NULL;
		free(original_note_id);
	}

	for (i = 0; i < number_of_echoes; i++)
	{
		int new_note = note + echo_note_interval_array[i];
		int new_velocity = MidiUtil_clamp((int)(velocity * echo_velocity_scale_array[i]), 0, 127);

		if (new_note >= 0 && new_note < 128)
		{
			ScheduledMessage_t scheduled_message = (ScheduledMessage_t)(malloc(sizeof (struct ScheduledMessage)));
			MidiUtilMessage_setNoteOff(scheduled_message->message, channel, new_note, new_velocity);
			MidiUtilAlarm_add(alarm, echo_beat_array[i] * 60000 / tempo_bpm, handle_echo_alarm, scheduled_message);
		}
	}
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
	for (i = 0; i < 128; i++) original_note_id_array[i] = NULL;

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


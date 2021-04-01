
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

struct Player
{
	MidiFileEvent_t event;
	long start_time_msecs;
	long stop_time_msecs;
	int base_channel;
	int base_note;
	int base_velocity;
};

typedef struct Player *Player_t;

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int trigger = 0;
static int gate = 0;
static MidiFile_t midi_file;
static float tempo_bpm = 100;
static MidiUtilLock_t lock;
static int note_velocity[16][128];
static int note_sustain[16][128];
static int channel_sustain[16];
static MidiUtilPointerArray_t trigger_on_players;
static MidiUtilPointerArray_t trigger_off_players;
static MidiUtilPointerArray_t gate_on_players;
static MidiUtilPointerArray_t gate_off_players;
static MidiUtilPointerArray_t combo_on_players;
static MidiUtilPointerArray_t combo_off_players;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --trigger ] [ --gate ] [ --note <beat> <duration beats> <note interval> <velocity> ] ... [ --loop <beats> ] [ --tempo <bpm, default 100> ]\n", program_name);
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

static void update_trigger_on_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(trigger_on_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(trigger_on_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteStartEvent(player->event))
				{
					int note = player->base_note + MidiFileNoteStartEvent_getNote(player->event) - 60;
					int velocity = player->base_velocity * MidiFileNoteStartEvent_getVelocity(player->event) / 127;
					if (note >= 0 && note < 128) send_note_on(player->base_channel, note, velocity);
				}

				player->event = MidiFileEvent_getNextEventInFile(player->event);
			}
			else
			{
				break;
			}
		}

		if (player->event == NULL)
		{
			MidiUtilPointerArray_remove(trigger_on_players, player_number--);
			free(player);
		}
	}
}

static void update_trigger_off_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(trigger_off_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(trigger_off_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				/* for trigger, we only care about note-ons in the sequence, even when deciding which note-offs to send */
				if (MidiFileEvent_isNoteStartEvent(player->event))
				{
					int note = player->base_note + MidiFileNoteStartEvent_getNote(player->event) - 60;
					int velocity = player->base_velocity * MidiFileNoteStartEvent_getVelocity(player->event) / 127;
					if (note >= 0 && note < 128) send_note_off(player->base_channel, note, velocity);
				}

				player->event = MidiFileEvent_getNextEventInFile(player->event);
			}
			else
			{
				break;
			}
		}

		if (player->event == NULL)
		{
			MidiUtilPointerArray_remove(trigger_off_players, player_number--);
			free(player);
		}
	}
}

static void update_gate_on_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(gate_on_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(gate_on_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteStartEvent(player->event))
				{
					int channel, base_note;

					for (channel = 0; channel < 16; channel++)
					{
						for (base_note = 0; base_note < 128; base_note++)
						{
							int base_velocity = note_velocity[channel][base_note];

							if (base_velocity > 0)
							{
								int note = base_note + MidiFileNoteStartEvent_getNote(player->event) - 60;
								int velocity = base_velocity * MidiFileNoteStartEvent_getVelocity(player->event) / 127;
								if (note >= 0 && note < 128) send_note_on(channel, note, velocity);
							}
						}
					}

					player->event = MidiFileEvent_getNextEventInFile(player->event);
				}
				else if (MidiFileEvent_isNoteEndEvent(player->event))
				{
					int channel, base_note;

					for (channel = 0; channel < 16; channel++)
					{
						for (base_note = 0; base_note < 128; base_note++)
						{
							int base_velocity = note_velocity[channel][base_note];

							if (base_velocity > 0)
							{
								int note = base_note + MidiFileNoteEndEvent_getNote(player->event) - 60;
								if (note >= 0 && note < 128) send_note_off(channel, note, 0);
							}
						}
					}

					player->event = MidiFileEvent_getNextEventInFile(player->event);
				}
				else if (MidiFileEvent_isMarkerEvent(player->event))
				{
					/* looping is like a note-off and immediate note-on */
					Player_t gate_off_player = (Player_t)(malloc(sizeof(struct Player)));
					gate_off_player->event = MidiFileEvent_getNextEventInFile(player->event);
					gate_off_player->start_time_msecs = player->start_time_msecs;
					gate_off_player->stop_time_msecs = current_time_msecs;
					MidiUtilPointerArray_add(gate_off_players, gate_off_player);

					player->event = MidiFile_getFirstEvent(midi_file);
					player->start_time_msecs = current_time_msecs;
				}
				else
				{
					player->event = MidiFileEvent_getNextEventInFile(player->event);
				}
			}
			else
			{
				break;
			}
		}

		if (player->event == NULL)
		{
			MidiUtilPointerArray_remove(gate_on_players, player_number--);
			free(player);
		}
	}
}

static void update_gate_off_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	/* play the corresponding note-off for each note that was on already, but no new note-ons nor extra note-offs */
	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(gate_off_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(gate_off_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteEndEvent(player->event))
				{
					MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(player->event);

					if (start_event != NULL)
					{
						long start_event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

						if (start_event_time_msecs <= player->stop_time_msecs)
						{
							int channel, base_note;

							for (channel = 0; channel < 16; channel++)
							{
								for (base_note = 0; base_note < 128; base_note++)
								{
									int base_velocity = note_velocity[channel][base_note];

									if (base_velocity > 0)
									{
										int note = base_note + MidiFileNoteEndEvent_getNote(player->event) - 60;
										if (note >= 0 && note < 128) send_note_off(channel, note, 0);
									}
								}
							}
						}
					}
				}

				player->event = MidiFileEvent_getNextEventInFile(player->event);
			}
			else
			{
				break;
			}
		}

		if (player->event == NULL)
		{
			MidiUtilPointerArray_remove(gate_off_players, player_number--);
			free(player);
		}
	}
}

static void update_combo_on_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(combo_on_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(combo_on_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteStartEvent(player->event))
				{
					int note = player->base_note + MidiFileNoteStartEvent_getNote(player->event) - 60;
					int velocity = player->base_velocity * MidiFileNoteStartEvent_getVelocity(player->event) / 127;
					if (note >= 0 && note < 128) send_note_on(player->base_channel, note, velocity);
					player->event = MidiFileEvent_getNextEventInFile(player->event);
				}
				else if (MidiFileEvent_isNoteEndEvent(player->event))
				{
					int note = player->base_note + MidiFileNoteEndEvent_getNote(player->event) - 60;
					int velocity = player->base_velocity * MidiFileNoteEndEvent_getVelocity(player->event) / 127;
					if (note >= 0 && note < 128) send_note_off(player->base_channel, note, velocity);
					player->event = MidiFileEvent_getNextEventInFile(player->event);
				}
				else if (MidiFileEvent_isMarkerEvent(player->event))
				{
					/* looping is like a note-off and immediate note-on */
					Player_t combo_off_player = (Player_t)(malloc(sizeof(struct Player)));
					combo_off_player->event = MidiFileEvent_getNextEventInFile(player->event);
					combo_off_player->start_time_msecs = player->start_time_msecs;
					combo_off_player->stop_time_msecs = current_time_msecs;
					combo_off_player->base_channel = player->base_channel;
					combo_off_player->base_note = player->base_note;
					combo_off_player->base_velocity = player->base_velocity;
					MidiUtilPointerArray_add(combo_off_players, combo_off_player);

					player->event = MidiFile_getFirstEvent(midi_file);
					player->start_time_msecs = current_time_msecs;
				}
				else
				{
					player->event = MidiFileEvent_getNextEventInFile(player->event);
				}
			}
			else
			{
				break;
			}
		}

		if (player->event == NULL)
		{
			MidiUtilPointerArray_remove(combo_on_players, player_number--);
			free(player);
		}
	}
}

static void update_combo_off_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	/* play the corresponding note-off for each note that was on already, but no new note-ons nor extra note-offs */
	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(combo_off_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(combo_off_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteEndEvent(player->event))
				{
					MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(player->event);

					if (start_event != NULL)
					{
						long start_event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

						if (start_event_time_msecs <= player->stop_time_msecs)
						{
							int note = player->base_note + MidiFileNoteEndEvent_getNote(player->event) - 60;
							if (note >= 0 && note < 128) send_note_off(player->base_channel, note, 0);
						}
					}
				}

				player->event = MidiFileEvent_getNextEventInFile(player->event);
			}
			else
			{
				break;
			}
		}

		if (player->event == NULL)
		{
			MidiUtilPointerArray_remove(combo_off_players, player_number--);
			free(player);
		}
	}
}

static void player_thread_main(void *user_data)
{
	while (1)
	{
		MidiUtilLock_lock(lock);
		update_trigger_on_players();
		update_trigger_off_players();
		update_gate_on_players();
		update_gate_off_players();
		update_combo_on_players();
		update_combo_off_players();
		MidiUtilLock_unlock(lock);
		MidiUtil_sleep(1); /* tune this for CPU busyness vs responsiveness */
	}
}

static void handle_virtual_note_on(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	if (trigger)
	{
		Player_t player = (Player_t)(malloc(sizeof(struct Player)));
		player->event = MidiFile_getFirstEvent(midi_file);
		player->start_time_msecs = current_time_msecs;
		player->base_channel = channel;
		player->base_note = note;
		player->base_velocity = velocity;

		if (gate)
		{
			MidiUtilPointerArray_add(combo_on_players, player);
		}
		else
		{
			MidiUtilPointerArray_add(trigger_on_players, player);
		}
	}
	else if (gate && (MidiUtilPointerArray_getSize(gate_on_players) == 0))
	{
		Player_t player = (Player_t)(malloc(sizeof(struct Player)));
		player->event = MidiFile_getFirstEvent(midi_file);
		player->start_time_msecs = current_time_msecs;
		MidiUtilPointerArray_add(gate_on_players, player);
	}
}

static void handle_virtual_note_off(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	if (trigger)
	{
		if (gate)
		{
			int player_number;

			for (player_number = 0; player_number < MidiUtilPointerArray_getSize(combo_on_players); player_number++)
			{
				Player_t player = (Player_t)(MidiUtilPointerArray_get(combo_on_players, player_number));

				/* turn off the first matching one */
				if ((player->base_channel == channel) && (player->base_note == note))
				{
					MidiUtilPointerArray_remove(combo_on_players, player_number);
					player->stop_time_msecs = current_time_msecs;
					MidiUtilPointerArray_add(combo_off_players, player);
					break;
				}
			}
		}
		else
		{
			Player_t player = (Player_t)(malloc(sizeof(struct Player)));
			player->event = MidiFile_getFirstEvent(midi_file);
			player->start_time_msecs = current_time_msecs;
			player->base_channel = channel;
			player->base_note = note;
			player->base_velocity = velocity;
			MidiUtilPointerArray_add(trigger_off_players, player);
		}
	}
	else if (gate)
	{
		int player_number;

		for (player_number = 0; player_number < MidiUtilPointerArray_getSize(gate_on_players); player_number++)
		{
			Player_t player = (Player_t)(MidiUtilPointerArray_get(gate_on_players, player_number));
			MidiFileEvent_t event;

			for (event = MidiFileEvent_getPreviousEventInFile(player->event); event != NULL; event = MidiFileEvent_getPreviousEventInFile(event))
			{
				if (MidiFileEvent_isNoteStartEvent(event))
				{
					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

					if (end_event != NULL)
					{
						long end_event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(end_event)) * 60000 / tempo_bpm);

						if (end_event_time_msecs >= current_time_msecs)
						{
							int on_note = note + MidiFileNoteStartEvent_getNote(event) - 60;
							if (on_note >= 0 && on_note < 128) send_note_off(channel, on_note, 0);
						}
					}
				}
			}
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

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	MidiUtilPointerArray_free(trigger_on_players);
	MidiUtilPointerArray_free(trigger_off_players);
	MidiUtilPointerArray_free(gate_on_players);
	MidiUtilPointerArray_free(gate_off_players);
	MidiUtilPointerArray_free(combo_on_players);
	MidiUtilPointerArray_free(combo_off_players);
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

	trigger_on_players = MidiUtilPointerArray_new(1024);
	trigger_off_players = MidiUtilPointerArray_new(1024);
	gate_on_players = MidiUtilPointerArray_new(1024);
	gate_off_players = MidiUtilPointerArray_new(1024);
	combo_on_players = MidiUtilPointerArray_new(1024);
	combo_off_players = MidiUtilPointerArray_new(1024);

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
		else if (strcmp(argv[i], "--trigger") == 0)
		{
			trigger = 1;
		}
		else if (strcmp(argv[i], "--gate") == 0)
		{
			gate = 1;
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
	MidiUtil_startThread(player_thread_main, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


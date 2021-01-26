
/*
 * This implementation treats the echo, arp, and seq patterns as MIDI sequences
 * which modulate the notes which are played interactively.  Players for these
 * sequences, each with its own notion of the "current event", are continuously
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
static float tempo_bpm = 100;
static MidiFile_t echo_midi_file = NULL;
static MidiFile_t arp_midi_file = NULL;
static MidiFile_t seq_midi_file = NULL;
static MidiUtilLock_t lock;
static int note_velocity[16][128];
static int note_sustain[16][128];
static int channel_sustain[16];
static MidiUtilPointerArray_t echo_on_players;
static MidiUtilPointerArray_t echo_off_players;
static MidiUtilPointerArray_t arp_on_players;
static MidiUtilPointerArray_t arp_off_players;
static MidiUtilPointerArray_t seq_on_players;
static MidiUtilPointerArray_t seq_off_players;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --tempo <bpm, default 100> ] [ --echo <beat> <note interval> <velocity> | --arp <beat> <duration beats> <note interval> <velocity> | --seq <beat> <duration beats> <note interval> <velocity> ] ... [ --arp-loop <beats> ] [ --seq-loop <beats, default 4> ]\n", program_name);
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

static void player_thread_main(void *user_data)
{
	while (1)
	{
		long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
		MidiUtilLock_lock(lock);

		if (echo_midi_file != NULL)
		{
			int echo_on_player_number, echo_off_player_number;

			for (echo_on_player_number = 0; echo_on_player_number < MidiUtilPointerArray_getSize(echo_on_players); echo_on_player_number++)
			{
				Player_t echo_on_player = (Player_t)(MidiUtilPointerArray_get(echo_on_players, echo_on_player_number));

				while (echo_on_player->event != NULL)
				{
					long event_time_msecs = echo_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(echo_midi_file, MidiFileEvent_getTick(echo_on_player->event)) * 60000 / tempo_bpm);

					if (event_time_msecs <= current_time_msecs)
					{
						if (MidiFileEvent_isNoteStartEvent(echo_on_player->event))
						{
							int note = echo_on_player->base_note + MidiFileNoteStartEvent_getNote(echo_on_player->event) - 60;
							int velocity = echo_on_player->base_velocity * MidiFileNoteStartEvent_getVelocity(echo_on_player->event) / 127;
							if (note >= 0 && note < 128) send_note_on(echo_on_player->base_channel, note, velocity);
						}

						echo_on_player->event = MidiFileEvent_getNextEventInFile(echo_on_player->event);
					}
					else
					{
						break;
					}
				}

				if (echo_on_player->event == NULL)
				{
					MidiUtilPointerArray_remove(echo_on_players, echo_on_player_number--);
					free(echo_on_player);
				}
			}

			for (echo_off_player_number = 0; echo_off_player_number < MidiUtilPointerArray_getSize(echo_off_players); echo_off_player_number++)
			{
				Player_t echo_off_player = (Player_t)(MidiUtilPointerArray_get(echo_off_players, echo_off_player_number));

				while (echo_off_player->event != NULL)
				{
					long event_time_msecs = echo_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(echo_midi_file, MidiFileEvent_getTick(echo_off_player->event)) * 60000 / tempo_bpm);

					if (event_time_msecs <= current_time_msecs)
					{
						/* we only care about note-ons in the echo sequence, even when deciding which note-offs to send */
						if (MidiFileEvent_isNoteStartEvent(echo_off_player->event))
						{
							int note = echo_off_player->base_note + MidiFileNoteStartEvent_getNote(echo_off_player->event) - 60;
							int velocity = echo_off_player->base_velocity * MidiFileNoteStartEvent_getVelocity(echo_off_player->event) / 127;
							if (note >= 0 && note < 128) send_note_off(echo_off_player->base_channel, note, velocity);
						}

						echo_off_player->event = MidiFileEvent_getNextEventInFile(echo_off_player->event);
					}
					else
					{
						break;
					}
				}

				if (echo_off_player->event == NULL)
				{
					MidiUtilPointerArray_remove(echo_off_players, echo_off_player_number--);
					free(echo_off_player);
				}
			}
		}

		if (arp_midi_file != NULL)
		{
			int arp_on_player_number, arp_off_player_number;

			for (arp_on_player_number = 0; arp_on_player_number < MidiUtilPointerArray_getSize(arp_on_players); arp_on_player_number++)
			{
				Player_t arp_on_player = (Player_t)(MidiUtilPointerArray_get(arp_on_players, arp_on_player_number));

				while (arp_on_player->event != NULL)
				{
					long event_time_msecs = arp_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(arp_midi_file, MidiFileEvent_getTick(arp_on_player->event)) * 60000 / tempo_bpm);

					if (event_time_msecs <= current_time_msecs)
					{
						if (MidiFileEvent_isNoteStartEvent(arp_on_player->event))
						{
							int note = arp_on_player->base_note + MidiFileNoteStartEvent_getNote(arp_on_player->event) - 60;
							int velocity = arp_on_player->base_velocity * MidiFileNoteStartEvent_getVelocity(arp_on_player->event) / 127;
							if (note >= 0 && note < 128) send_note_on(arp_on_player->base_channel, note, velocity);
							arp_on_player->event = MidiFileEvent_getNextEventInFile(arp_on_player->event);
						}
						else if (MidiFileEvent_isNoteEndEvent(arp_on_player->event))
						{
							int note = arp_on_player->base_note + MidiFileNoteEndEvent_getNote(arp_on_player->event) - 60;
							int velocity = arp_on_player->base_velocity * MidiFileNoteEndEvent_getVelocity(arp_on_player->event) / 127;
							if (note >= 0 && note < 128) send_note_off(arp_on_player->base_channel, note, velocity);
							arp_on_player->event = MidiFileEvent_getNextEventInFile(arp_on_player->event);
						}
						else if (MidiFileEvent_isMarkerEvent(arp_on_player->event))
						{
							/* looping the arp is like a note-off and immediate note-on */
							Player_t arp_off_player = (Player_t)(malloc(sizeof(struct Player)));
							arp_off_player->event = MidiFileEvent_getNextEventInFile(arp_on_player->event);
							arp_off_player->start_time_msecs = arp_on_player->start_time_msecs;
							arp_off_player->stop_time_msecs = current_time_msecs;
							arp_off_player->base_channel = arp_on_player->base_channel;
							arp_off_player->base_note = arp_on_player->base_note;
							arp_off_player->base_velocity = arp_on_player->base_velocity;
							MidiUtilPointerArray_add(arp_off_players, arp_off_player);

							arp_on_player->event = MidiFile_getFirstEvent(arp_midi_file);
							arp_on_player->start_time_msecs = current_time_msecs;
						}
						else
						{
							arp_on_player->event = MidiFileEvent_getNextEventInFile(arp_on_player->event);
						}
					}
					else
					{
						break;
					}
				}

				if (arp_on_player->event == NULL)
				{
					MidiUtilPointerArray_remove(arp_on_players, arp_on_player_number--);
					free(arp_on_player);
				}
			}

			/* play the corresponding note-off for each note that was on already, but no new note-ons nor extra note-offs */
			for (arp_off_player_number = 0; arp_off_player_number < MidiUtilPointerArray_getSize(arp_off_players); arp_off_player_number++)
			{
				Player_t arp_off_player = (Player_t)(MidiUtilPointerArray_get(arp_off_players, arp_off_player_number));

				while (arp_off_player->event != NULL)
				{
					long event_time_msecs = arp_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(arp_midi_file, MidiFileEvent_getTick(arp_off_player->event)) * 60000 / tempo_bpm);

					if (event_time_msecs <= current_time_msecs)
					{
						if (MidiFileEvent_isNoteEndEvent(arp_off_player->event))
						{
							MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(arp_off_player->event);

							if (start_event != NULL)
							{
								long start_event_time_msecs = arp_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(arp_midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

								if (start_event_time_msecs <= arp_off_player->stop_time_msecs)
								{
									int note = arp_off_player->base_note + MidiFileNoteEndEvent_getNote(arp_off_player->event) - 60;
									if (note >= 0 && note < 128) send_note_off(arp_off_player->base_channel, note, 0);
								}
							}
						}

						arp_off_player->event = MidiFileEvent_getNextEventInFile(arp_off_player->event);
					}
					else
					{
						break;
					}
				}

				if (arp_off_player->event == NULL)
				{
					MidiUtilPointerArray_remove(arp_off_players, arp_off_player_number--);
					free(arp_off_player);
				}
			}
		}

		if (seq_midi_file != NULL)
		{
			int seq_on_player_number, seq_off_player_number;

			for (seq_on_player_number = 0; seq_on_player_number < MidiUtilPointerArray_getSize(seq_on_players); seq_on_player_number++)
			{
				Player_t seq_on_player = (Player_t)(MidiUtilPointerArray_get(seq_on_players, seq_on_player_number));

				while (seq_on_player->event != NULL)
				{
					long event_time_msecs = seq_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(seq_midi_file, MidiFileEvent_getTick(seq_on_player->event)) * 60000 / tempo_bpm);

					if (event_time_msecs <= current_time_msecs)
					{
						if (MidiFileEvent_isNoteStartEvent(seq_on_player->event))
						{
							int channel, base_note;

							for (channel = 0; channel < 16; channel++)
							{
								for (base_note = 0; base_note < 128; base_note++)
								{
									int base_velocity = note_velocity[channel][base_note];

									if (base_velocity > 0)
									{
										int note = base_note + MidiFileNoteStartEvent_getNote(seq_on_player->event) - 60;
										int velocity = base_velocity * MidiFileNoteStartEvent_getVelocity(seq_on_player->event) / 127;
										if (note >= 0 && note < 128) send_note_on(channel, note, velocity);
									}
								}
							}

							seq_on_player->event = MidiFileEvent_getNextEventInFile(seq_on_player->event);
						}
						else if (MidiFileEvent_isNoteEndEvent(seq_on_player->event))
						{
							int channel, base_note;

							for (channel = 0; channel < 16; channel++)
							{
								for (base_note = 0; base_note < 128; base_note++)
								{
									int base_velocity = note_velocity[channel][base_note];

									if (base_velocity > 0)
									{
										int note = base_note + MidiFileNoteEndEvent_getNote(seq_on_player->event) - 60;
										if (note >= 0 && note < 128) send_note_off(channel, note, 0);
									}
								}
							}

							seq_on_player->event = MidiFileEvent_getNextEventInFile(seq_on_player->event);
						}
						else if (MidiFileEvent_isMarkerEvent(seq_on_player->event))
						{
							/* looping the seq is like a note-off and immediate note-on */
							Player_t seq_off_player = (Player_t)(malloc(sizeof(struct Player)));
							seq_off_player->event = MidiFileEvent_getNextEventInFile(seq_on_player->event);
							seq_off_player->start_time_msecs = seq_on_player->start_time_msecs;
							seq_off_player->stop_time_msecs = current_time_msecs;
							MidiUtilPointerArray_add(seq_off_players, seq_off_player);

							seq_on_player->event = MidiFile_getFirstEvent(seq_midi_file);
							seq_on_player->start_time_msecs = current_time_msecs;
						}
						else
						{
							seq_on_player->event = MidiFileEvent_getNextEventInFile(seq_on_player->event);
						}
					}
					else
					{
						break;
					}
				}

				if (seq_on_player->event == NULL)
				{
					MidiUtilPointerArray_remove(seq_on_players, seq_on_player_number--);
					free(seq_on_player);
				}
			}

			/* play the corresponding note-off for each note that was on already, but no new note-ons nor extra note-offs */
			for (seq_off_player_number = 0; seq_off_player_number < MidiUtilPointerArray_getSize(seq_off_players); seq_off_player_number++)
			{
				Player_t seq_off_player = (Player_t)(MidiUtilPointerArray_get(seq_off_players, seq_off_player_number));

				while (seq_off_player->event != NULL)
				{
					long event_time_msecs = seq_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(seq_midi_file, MidiFileEvent_getTick(seq_off_player->event)) * 60000 / tempo_bpm);

					if (event_time_msecs <= current_time_msecs)
					{
						if (MidiFileEvent_isNoteEndEvent(seq_off_player->event))
						{
							MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(seq_off_player->event);

							if (start_event != NULL)
							{
								long start_event_time_msecs = seq_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(seq_midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

								if (start_event_time_msecs <= seq_off_player->stop_time_msecs)
								{
									int channel, base_note;

									for (channel = 0; channel < 16; channel++)
									{
										for (base_note = 0; base_note < 128; base_note++)
										{
											int base_velocity = note_velocity[channel][base_note];

											if (base_velocity > 0)
											{
												int note = base_note + MidiFileNoteEndEvent_getNote(seq_off_player->event) - 60;
												if (note >= 0 && note < 128) send_note_off(channel, note, 0);
											}
										}
									}
								}
							}
						}

						seq_off_player->event = MidiFileEvent_getNextEventInFile(seq_off_player->event);
					}
					else
					{
						break;
					}
				}

				if (seq_off_player->event == NULL)
				{
					MidiUtilPointerArray_remove(seq_off_players, seq_off_player_number--);
					free(seq_off_player);
				}
			}
		}

		MidiUtilLock_unlock(lock);
		MidiUtil_sleep(5); /* tune this for CPU busyness vs responsiveness */
	}
}

static void handle_virtual_note_on(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	if (echo_midi_file != NULL)
	{
		Player_t echo_on_player = (Player_t)(malloc(sizeof(struct Player)));
		echo_on_player->event = MidiFile_getFirstEvent(echo_midi_file);
		echo_on_player->start_time_msecs = current_time_msecs;
		echo_on_player->base_channel = channel;
		echo_on_player->base_note = note;
		echo_on_player->base_velocity = velocity;
		MidiUtilPointerArray_add(echo_on_players, echo_on_player);
	}

	if (arp_midi_file != NULL)
	{
		Player_t arp_on_player = (Player_t)(malloc(sizeof(struct Player)));
		arp_on_player->event = MidiFile_getFirstEvent(arp_midi_file);
		arp_on_player->start_time_msecs = current_time_msecs;
		arp_on_player->base_channel = channel;
		arp_on_player->base_note = note;
		arp_on_player->base_velocity = velocity;
		MidiUtilPointerArray_add(arp_on_players, arp_on_player);
	}

	if ((seq_midi_file != NULL) && (MidiUtilPointerArray_getSize(seq_on_players) == 0))
	{
		Player_t seq_on_player = (Player_t)(malloc(sizeof(struct Player)));
		seq_on_player->event = MidiFile_getFirstEvent(seq_midi_file);
		seq_on_player->start_time_msecs = current_time_msecs;
		MidiUtilPointerArray_add(seq_on_players, seq_on_player);
	}
}

static void handle_virtual_note_off(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	if (echo_midi_file != NULL)
	{
		Player_t echo_off_player = (Player_t)(malloc(sizeof(struct Player)));
		echo_off_player->event = MidiFile_getFirstEvent(echo_midi_file);
		echo_off_player->start_time_msecs = current_time_msecs;
		echo_off_player->base_channel = channel;
		echo_off_player->base_note = note;
		echo_off_player->base_velocity = velocity;
		MidiUtilPointerArray_add(echo_off_players, echo_off_player);
	}

	if (arp_midi_file != NULL)
	{
		int arp_on_player_number;

		for (arp_on_player_number = 0; arp_on_player_number < MidiUtilPointerArray_getSize(arp_on_players); arp_on_player_number++)
		{
			Player_t arp_on_player = (Player_t)(MidiUtilPointerArray_get(arp_on_players, arp_on_player_number));

			/* turn off the first matching one */
			if ((arp_on_player->base_channel == channel) && (arp_on_player->base_note == note))
			{
				MidiUtilPointerArray_remove(arp_on_players, arp_on_player_number);
				arp_on_player->stop_time_msecs = current_time_msecs;
				MidiUtilPointerArray_add(arp_off_players, arp_on_player);
				break;
			}
		}
	}

	if (seq_midi_file != NULL)
	{
		int seq_on_player_number;

		for (seq_on_player_number = 0; seq_on_player_number < MidiUtilPointerArray_getSize(seq_on_players); seq_on_player_number++)
		{
			Player_t seq_on_player = (Player_t)(MidiUtilPointerArray_get(seq_on_players, seq_on_player_number));
			MidiFileEvent_t event;

			for (event = MidiFileEvent_getPreviousEventInFile(seq_on_player->event); event != NULL; event = MidiFileEvent_getPreviousEventInFile(event))
			{
				if (MidiFileEvent_isNoteStartEvent(event))
				{
					MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

					if (end_event != NULL)
					{
						long end_event_time_msecs = seq_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(seq_midi_file, MidiFileEvent_getTick(end_event)) * 60000 / tempo_bpm);

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
	MidiUtilPointerArray_free(echo_on_players);
	MidiUtilPointerArray_free(echo_off_players);
	MidiUtilPointerArray_free(arp_on_players);
	MidiUtilPointerArray_free(arp_off_players);
	MidiUtilPointerArray_free(seq_on_players);
	MidiUtilPointerArray_free(seq_off_players);
	MidiUtilLock_free(lock);
	if (echo_midi_file != NULL) MidiFile_free(echo_midi_file);
	if (arp_midi_file != NULL) MidiFile_free(arp_midi_file);
	if (seq_midi_file != NULL) MidiFile_free(seq_midi_file);
}

int main(int argc, char **argv)
{
	float arp_loop_beats = -1;
	float seq_loop_beats = 4;
	int i, j;

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

	echo_on_players = MidiUtilPointerArray_new(1024);
	echo_off_players = MidiUtilPointerArray_new(1024);
	arp_on_players = MidiUtilPointerArray_new(1024);
	arp_off_players = MidiUtilPointerArray_new(1024);
	seq_on_players = MidiUtilPointerArray_new(1024);
	seq_off_players = MidiUtilPointerArray_new(1024);

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
		else if (strcmp(argv[i], "--echo") == 0)
		{
			float beat;
			int note_interval;
			int velocity;
			if (++i == argc) usage(argv[0]);
			beat = atof(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_interval = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			velocity = atoi(argv[i]);
			if (echo_midi_file == NULL) echo_midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
			MidiFileTrack_createNoteOnEvent(MidiFile_getTrackByNumber(echo_midi_file, 1, 1), MidiFile_getTickFromBeat(echo_midi_file, beat), 0, 60 + note_interval, velocity);
		}
		else if (strcmp(argv[i], "--arp") == 0)
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
			if (arp_midi_file == NULL) arp_midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
			MidiFileTrack_createNoteStartAndEndEvents(MidiFile_getTrackByNumber(arp_midi_file, 1, 1), MidiFile_getTickFromBeat(arp_midi_file, beat), MidiFile_getTickFromBeat(arp_midi_file, beat + duration_beats), 0, 60 + note_interval, velocity, 0);
		}
		else if (strcmp(argv[i], "--seq") == 0)
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
			if (seq_midi_file == NULL) seq_midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
			MidiFileTrack_createNoteStartAndEndEvents(MidiFile_getTrackByNumber(seq_midi_file, 1, 1), MidiFile_getTickFromBeat(seq_midi_file, beat), MidiFile_getTickFromBeat(seq_midi_file, beat + duration_beats), 0, 60 + note_interval, velocity, 0);
		}
		else if (strcmp(argv[i], "--arp-loop") == 0)
		{
			if (++i == argc) usage(argv[0]);
			arp_loop_beats = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--seq-loop") == 0)
		{
			if (++i == argc) usage(argv[0]);
			seq_loop_beats = atof(argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL)) usage(argv[0]);
	if ((arp_midi_file != NULL) && (arp_loop_beats >= 0)) MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(arp_midi_file, 1, 1), MidiFile_getTickFromBeat(arp_midi_file, arp_loop_beats), "loop");
	if ((seq_midi_file != NULL) && (seq_loop_beats >= 0)) MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(seq_midi_file, 1, 1), MidiFile_getTickFromBeat(seq_midi_file, seq_loop_beats), "loop");
	MidiUtil_startThread(player_thread_main, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}



/*
 * This implementation treats the echo, trigger, and sync patterns a MIDI
 * sequence which modulates the notes that are played interactively.  Players
 * for these sequences, each with its own notion of the "current event", are
 * continuously created and destroyed in response to the notes being played
 * interactively.  Due to the large number of simultaneous players, it uses a
 * game loop rather than alarms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

typedef enum
{
	MODE_NONE,
	MODE_ECHO,
	MODE_TRIGGER,
	MODE_GATE
}
Mode_t;

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
static Mode_t mode = MODE_NONE;
static MidiFile_t midi_file;
static float tempo_bpm = 100;
static MidiUtilLock_t lock;
static int note_velocity[16][128];
static int note_sustain[16][128];
static int channel_sustain[16];
static MidiUtilPointerArray_t echo_on_players;
static MidiUtilPointerArray_t echo_off_players;
static MidiUtilPointerArray_t trigger_on_players;
static MidiUtilPointerArray_t trigger_off_players;
static MidiUtilPointerArray_t gate_on_players;
static MidiUtilPointerArray_t gate_off_players;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> ( --echo | --trigger | --gate ) [ --note <beat> <duration beats> <note interval> <velocity> ] ... [ --loop <beats> ] [ --tempo <bpm, default 100> ]\n", program_name);
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

static void update_echo_on_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(echo_on_players); player_number++)
	{
		Player_t echo_on_player = (Player_t)(MidiUtilPointerArray_get(echo_on_players, player_number));

		while (echo_on_player->event != NULL)
		{
			long event_time_msecs = echo_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(echo_on_player->event)) * 60000 / tempo_bpm);

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
			MidiUtilPointerArray_remove(echo_on_players, player_number--);
			free(echo_on_player);
		}
	}
}

static void update_echo_off_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(echo_off_players); player_number++)
	{
		Player_t echo_off_player = (Player_t)(MidiUtilPointerArray_get(echo_off_players, player_number));

		while (echo_off_player->event != NULL)
		{
			long event_time_msecs = echo_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(echo_off_player->event)) * 60000 / tempo_bpm);

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
			MidiUtilPointerArray_remove(echo_off_players, player_number--);
			free(echo_off_player);
		}
	}
}

static void update_trigger_on_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(trigger_on_players); player_number++)
	{
		Player_t trigger_on_player = (Player_t)(MidiUtilPointerArray_get(trigger_on_players, player_number));

		while (trigger_on_player->event != NULL)
		{
			long event_time_msecs = trigger_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(trigger_on_player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteStartEvent(trigger_on_player->event))
				{
					int note = trigger_on_player->base_note + MidiFileNoteStartEvent_getNote(trigger_on_player->event) - 60;
					int velocity = trigger_on_player->base_velocity * MidiFileNoteStartEvent_getVelocity(trigger_on_player->event) / 127;
					if (note >= 0 && note < 128) send_note_on(trigger_on_player->base_channel, note, velocity);
					trigger_on_player->event = MidiFileEvent_getNextEventInFile(trigger_on_player->event);
				}
				else if (MidiFileEvent_isNoteEndEvent(trigger_on_player->event))
				{
					int note = trigger_on_player->base_note + MidiFileNoteEndEvent_getNote(trigger_on_player->event) - 60;
					int velocity = trigger_on_player->base_velocity * MidiFileNoteEndEvent_getVelocity(trigger_on_player->event) / 127;
					if (note >= 0 && note < 128) send_note_off(trigger_on_player->base_channel, note, velocity);
					trigger_on_player->event = MidiFileEvent_getNextEventInFile(trigger_on_player->event);
				}
				else if (MidiFileEvent_isMarkerEvent(trigger_on_player->event))
				{
					/* looping the arp is like a note-off and immediate note-on */
					Player_t trigger_off_player = (Player_t)(malloc(sizeof(struct Player)));
					trigger_off_player->event = MidiFileEvent_getNextEventInFile(trigger_on_player->event);
					trigger_off_player->start_time_msecs = trigger_on_player->start_time_msecs;
					trigger_off_player->stop_time_msecs = current_time_msecs;
					trigger_off_player->base_channel = trigger_on_player->base_channel;
					trigger_off_player->base_note = trigger_on_player->base_note;
					trigger_off_player->base_velocity = trigger_on_player->base_velocity;
					MidiUtilPointerArray_add(trigger_off_players, trigger_off_player);

					trigger_on_player->event = MidiFile_getFirstEvent(midi_file);
					trigger_on_player->start_time_msecs = current_time_msecs;
				}
				else
				{
					trigger_on_player->event = MidiFileEvent_getNextEventInFile(trigger_on_player->event);
				}
			}
			else
			{
				break;
			}
		}

		if (trigger_on_player->event == NULL)
		{
			MidiUtilPointerArray_remove(trigger_on_players, player_number--);
			free(trigger_on_player);
		}
	}
}

static void update_trigger_off_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	/* play the corresponding note-off for each note that was on already, but no new note-ons nor extra note-offs */
	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(trigger_off_players); player_number++)
	{
		Player_t trigger_off_player = (Player_t)(MidiUtilPointerArray_get(trigger_off_players, player_number));

		while (trigger_off_player->event != NULL)
		{
			long event_time_msecs = trigger_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(trigger_off_player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteEndEvent(trigger_off_player->event))
				{
					MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(trigger_off_player->event);

					if (start_event != NULL)
					{
						long start_event_time_msecs = trigger_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

						if (start_event_time_msecs <= trigger_off_player->stop_time_msecs)
						{
							int note = trigger_off_player->base_note + MidiFileNoteEndEvent_getNote(trigger_off_player->event) - 60;
							if (note >= 0 && note < 128) send_note_off(trigger_off_player->base_channel, note, 0);
						}
					}
				}

				trigger_off_player->event = MidiFileEvent_getNextEventInFile(trigger_off_player->event);
			}
			else
			{
				break;
			}
		}

		if (trigger_off_player->event == NULL)
		{
			MidiUtilPointerArray_remove(trigger_off_players, player_number--);
			free(trigger_off_player);
		}
	}
}

static void update_gate_on_players(void)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(gate_on_players); player_number++)
	{
		Player_t gate_on_player = (Player_t)(MidiUtilPointerArray_get(gate_on_players, player_number));

		while (gate_on_player->event != NULL)
		{
			long event_time_msecs = gate_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(gate_on_player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteStartEvent(gate_on_player->event))
				{
					int channel, base_note;

					for (channel = 0; channel < 16; channel++)
					{
						for (base_note = 0; base_note < 128; base_note++)
						{
							int base_velocity = note_velocity[channel][base_note];

							if (base_velocity > 0)
							{
								int note = base_note + MidiFileNoteStartEvent_getNote(gate_on_player->event) - 60;
								int velocity = base_velocity * MidiFileNoteStartEvent_getVelocity(gate_on_player->event) / 127;
								if (note >= 0 && note < 128) send_note_on(channel, note, velocity);
							}
						}
					}

					gate_on_player->event = MidiFileEvent_getNextEventInFile(gate_on_player->event);
				}
				else if (MidiFileEvent_isNoteEndEvent(gate_on_player->event))
				{
					int channel, base_note;

					for (channel = 0; channel < 16; channel++)
					{
						for (base_note = 0; base_note < 128; base_note++)
						{
							int base_velocity = note_velocity[channel][base_note];

							if (base_velocity > 0)
							{
								int note = base_note + MidiFileNoteEndEvent_getNote(gate_on_player->event) - 60;
								if (note >= 0 && note < 128) send_note_off(channel, note, 0);
							}
						}
					}

					gate_on_player->event = MidiFileEvent_getNextEventInFile(gate_on_player->event);
				}
				else if (MidiFileEvent_isMarkerEvent(gate_on_player->event))
				{
					/* looping the seq is like a note-off and immediate note-on */
					Player_t gate_off_player = (Player_t)(malloc(sizeof(struct Player)));
					gate_off_player->event = MidiFileEvent_getNextEventInFile(gate_on_player->event);
					gate_off_player->start_time_msecs = gate_on_player->start_time_msecs;
					gate_off_player->stop_time_msecs = current_time_msecs;
					MidiUtilPointerArray_add(gate_off_players, gate_off_player);

					gate_on_player->event = MidiFile_getFirstEvent(midi_file);
					gate_on_player->start_time_msecs = current_time_msecs;
				}
				else
				{
					gate_on_player->event = MidiFileEvent_getNextEventInFile(gate_on_player->event);
				}
			}
			else
			{
				break;
			}
		}

		if (gate_on_player->event == NULL)
		{
			MidiUtilPointerArray_remove(gate_on_players, player_number--);
			free(gate_on_player);
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
		Player_t gate_off_player = (Player_t)(MidiUtilPointerArray_get(gate_off_players, player_number));

		while (gate_off_player->event != NULL)
		{
			long event_time_msecs = gate_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(gate_off_player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				if (MidiFileEvent_isNoteEndEvent(gate_off_player->event))
				{
					MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(gate_off_player->event);

					if (start_event != NULL)
					{
						long start_event_time_msecs = gate_off_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

						if (start_event_time_msecs <= gate_off_player->stop_time_msecs)
						{
							int channel, base_note;

							for (channel = 0; channel < 16; channel++)
							{
								for (base_note = 0; base_note < 128; base_note++)
								{
									int base_velocity = note_velocity[channel][base_note];

									if (base_velocity > 0)
									{
										int note = base_note + MidiFileNoteEndEvent_getNote(gate_off_player->event) - 60;
										if (note >= 0 && note < 128) send_note_off(channel, note, 0);
									}
								}
							}
						}
					}
				}

				gate_off_player->event = MidiFileEvent_getNextEventInFile(gate_off_player->event);
			}
			else
			{
				break;
			}
		}

		if (gate_off_player->event == NULL)
		{
			MidiUtilPointerArray_remove(gate_off_players, player_number--);
			free(gate_off_player);
		}
	}
}

static void player_thread_main(void *user_data)
{
	while (1)
	{
		MidiUtilLock_lock(lock);
		update_echo_on_players();
		update_echo_off_players();
		update_trigger_on_players();
		update_trigger_off_players();
		update_gate_on_players();
		update_gate_off_players();
		MidiUtilLock_unlock(lock);
		MidiUtil_sleep(1); /* tune this for CPU busyness vs responsiveness */
	}
}

static void handle_virtual_note_on(int channel, int note, int velocity)
{
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	switch (mode)
	{
		case MODE_ECHO:
		{
			Player_t echo_on_player = (Player_t)(malloc(sizeof(struct Player)));
			echo_on_player->event = MidiFile_getFirstEvent(midi_file);
			echo_on_player->start_time_msecs = current_time_msecs;
			echo_on_player->base_channel = channel;
			echo_on_player->base_note = note;
			echo_on_player->base_velocity = velocity;
			MidiUtilPointerArray_add(echo_on_players, echo_on_player);
			break;
		}
		case MODE_TRIGGER:
		{
			Player_t trigger_on_player = (Player_t)(malloc(sizeof(struct Player)));
			trigger_on_player->event = MidiFile_getFirstEvent(midi_file);
			trigger_on_player->start_time_msecs = current_time_msecs;
			trigger_on_player->base_channel = channel;
			trigger_on_player->base_note = note;
			trigger_on_player->base_velocity = velocity;
			MidiUtilPointerArray_add(trigger_on_players, trigger_on_player);
			break;
		}
		case MODE_GATE:
		{
			if (MidiUtilPointerArray_getSize(gate_on_players) == 0)
			{
				Player_t gate_on_player = (Player_t)(malloc(sizeof(struct Player)));
				gate_on_player->event = MidiFile_getFirstEvent(midi_file);
				gate_on_player->start_time_msecs = current_time_msecs;
				MidiUtilPointerArray_add(gate_on_players, gate_on_player);
			}

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
			Player_t echo_off_player = (Player_t)(malloc(sizeof(struct Player)));
			echo_off_player->event = MidiFile_getFirstEvent(midi_file);
			echo_off_player->start_time_msecs = current_time_msecs;
			echo_off_player->base_channel = channel;
			echo_off_player->base_note = note;
			echo_off_player->base_velocity = velocity;
			MidiUtilPointerArray_add(echo_off_players, echo_off_player);
			break;
		}
		case MODE_TRIGGER:
		{
			int trigger_on_player_number;

			for (trigger_on_player_number = 0; trigger_on_player_number < MidiUtilPointerArray_getSize(trigger_on_players); trigger_on_player_number++)
			{
				Player_t trigger_on_player = (Player_t)(MidiUtilPointerArray_get(trigger_on_players, trigger_on_player_number));

				/* turn off the first matching one */
				if ((trigger_on_player->base_channel == channel) && (trigger_on_player->base_note == note))
				{
					MidiUtilPointerArray_remove(trigger_on_players, trigger_on_player_number);
					trigger_on_player->stop_time_msecs = current_time_msecs;
					MidiUtilPointerArray_add(trigger_off_players, trigger_on_player);
					break;
				}
			}

			break;
		}
		case MODE_GATE:
		{
			int gate_on_player_number;

			for (gate_on_player_number = 0; gate_on_player_number < MidiUtilPointerArray_getSize(gate_on_players); gate_on_player_number++)
			{
				Player_t gate_on_player = (Player_t)(MidiUtilPointerArray_get(gate_on_players, gate_on_player_number));
				MidiFileEvent_t event;

				for (event = MidiFileEvent_getPreviousEventInFile(gate_on_player->event); event != NULL; event = MidiFileEvent_getPreviousEventInFile(event))
				{
					if (MidiFileEvent_isNoteStartEvent(event))
					{
						MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

						if (end_event != NULL)
						{
							long end_event_time_msecs = gate_on_player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(end_event)) * 60000 / tempo_bpm);

							if (end_event_time_msecs >= current_time_msecs)
							{
								int on_note = note + MidiFileNoteStartEvent_getNote(event) - 60;
								if (on_note >= 0 && on_note < 128) send_note_off(channel, on_note, 0);
							}
						}
					}
				}
			}

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

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	MidiUtilPointerArray_free(echo_on_players);
	MidiUtilPointerArray_free(echo_off_players);
	MidiUtilPointerArray_free(trigger_on_players);
	MidiUtilPointerArray_free(trigger_off_players);
	MidiUtilPointerArray_free(gate_on_players);
	MidiUtilPointerArray_free(gate_off_players);
	MidiUtilLock_free(lock);
	MidiFile_free(midi_file);
}

int main(int argc, char **argv)
{
	float loop_beats = -1;
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

	echo_on_players = MidiUtilPointerArray_new(1024);
	echo_off_players = MidiUtilPointerArray_new(1024);
	trigger_on_players = MidiUtilPointerArray_new(1024);
	trigger_off_players = MidiUtilPointerArray_new(1024);
	gate_on_players = MidiUtilPointerArray_new(1024);
	gate_off_players = MidiUtilPointerArray_new(1024);

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
		else if (strcmp(argv[i], "--trigger") == 0)
		{
			mode = MODE_TRIGGER;
		}
		else if (strcmp(argv[i], "--gate") == 0)
		{
			mode = MODE_GATE;
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
			if (++i == argc) usage(argv[0]);
			loop_beats = atof(argv[i]);
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

	if ((midi_in == NULL) || (midi_out == NULL) || (mode == MODE_NONE)) usage(argv[0]);
	if ((mode == MODE_GATE) && (loop_beats < 0)) loop_beats = 4;
	if (loop_beats >= 0) MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(midi_file, 1, 1), MidiFile_getTickFromBeat(midi_file, loop_beats), "loop");
	MidiUtil_startThread(player_thread_main, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


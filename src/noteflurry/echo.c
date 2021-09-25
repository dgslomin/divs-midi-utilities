
/* echo mode plays the sequence start to finish for each note played */

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

static MidiUtilPointerArray_t on_players;
static MidiUtilPointerArray_t off_players;

void echo_init(void)
{
	on_players = MidiUtilPointerArray_new(1024);
	off_players = MidiUtilPointerArray_new(1024);
}

void echo_cleanup(void)
{
	MidiUtilPointerArray_free(on_players);
	MidiUtilPointerArray_free(off_players);
}

void echo_note_on(long current_time_msecs, int channel, int note, int velocity)
{
	Player_t player = (Player_t)(malloc(sizeof(struct Player)));
	player->event = MidiFile_getFirstEvent(midi_file);
	player->start_time_msecs = current_time_msecs;
	player->base_channel = channel;
	player->base_note = note;
	player->base_velocity = velocity;
	MidiUtilPointerArray_add(on_players, player);
}

void echo_note_off(long current_time_msecs, int channel, int note, int velocity)
{
	Player_t player = (Player_t)(malloc(sizeof(struct Player)));
	player->event = MidiFile_getFirstEvent(midi_file);
	player->start_time_msecs = current_time_msecs;
	player->base_channel = channel;
	player->base_note = note;
	player->base_velocity = velocity;
	MidiUtilPointerArray_add(off_players, player);
}

static void update_on_players(long current_time_msecs)
{
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(on_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(on_players, player_number));

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
			MidiUtilPointerArray_remove(on_players, player_number--);
			free(player);
		}
	}
}

static void update_off_players(long current_time_msecs)
{
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(off_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(off_players, player_number));

		while (player->event != NULL)
		{
			long event_time_msecs = player->start_time_msecs + (long)(MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(player->event)) * 60000 / tempo_bpm);

			if (event_time_msecs <= current_time_msecs)
			{
				/* for echo, we only care about note-ons in the sequence, even when deciding which note-offs to send */
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
			MidiUtilPointerArray_remove(off_players, player_number--);
			free(player);
		}
	}
}

void echo_update_players(long current_time_msecs)
{
	update_on_players(current_time_msecs);
	update_off_players(current_time_msecs);
}


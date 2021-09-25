
/* seq mode silently loops through the sequence, unmuting when notes are played */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>
#include "noteflurry.h"
#include "seq.h"

static MidiUtilPointerArray_t on_players;
static MidiUtilPointerArray_t off_players;

void seq_init(void)
{
	on_players = MidiUtilPointerArray_new(1024);
	off_players = MidiUtilPointerArray_new(1024);
}

void seq_cleanup(void)
{
	MidiUtilPointerArray_free(on_players);
	MidiUtilPointerArray_free(off_players);
}

void seq_note_on(long current_time_msecs, int channel, int note, int velocity)
{
	if (MidiUtilPointerArray_getSize(on_players) == 0)
	{
		Player_t player = (Player_t)(malloc(sizeof(struct Player)));
		player->event = MidiFile_getFirstEvent(midi_file);
		player->start_time_msecs = current_time_msecs;
		MidiUtilPointerArray_add(on_players, player);
	}
}

void seq_note_off(long current_time_msecs, int channel, int note, int velocity)
{
	int player_number;

	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(on_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(on_players, player_number));
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
					Player_t off_player = (Player_t)(malloc(sizeof(struct Player)));
					off_player->event = MidiFileEvent_getNextEventInFile(player->event);
					off_player->start_time_msecs = player->start_time_msecs;
					off_player->stop_time_msecs = current_time_msecs;
					MidiUtilPointerArray_add(off_players, off_player);

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
			MidiUtilPointerArray_remove(on_players, player_number--);
			free(player);
		}
	}
}

static void update_off_players(long current_time_msecs)
{
	int player_number;

	/* play the corresponding note-off for each note that was on already, but no new note-ons nor extra note-offs */
	for (player_number = 0; player_number < MidiUtilPointerArray_getSize(off_players); player_number++)
	{
		Player_t player = (Player_t)(MidiUtilPointerArray_get(off_players, player_number));

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
			MidiUtilPointerArray_remove(off_players, player_number--);
			free(player);
		}
	}
}

void seq_update_players(long current_time_msecs)
{
	update_on_players(current_time_msecs);
	update_off_players(current_time_msecs);
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <midifile-player.h>
#include <midifile-player-support.h>

struct MidiFilePlayer
{
	MidiFileEventVisitorCallback_t visitor_callback;
	void *visitor_callback_user_data;
	MidiFile_t midi_file;
	int is_running;
	int should_shutdown;
	long relative_start_time;
	long absolute_start_time;
	MidiFileEvent_t event;
	MidiFilePlayerWaitLock_t wait_lock;
};

static void stop_held_notes(MidiFilePlayer_t player)
{
	MidiFile_t midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	long tick = MidiFilePlayer_getTick(player);
	MidiFileTrack_t track = MidiFile_getTrackByNumber(midi_file, 1, 1);
	MidiFileEvent_t note_event = MidiFileTrack_createNoteOffEvent(track, tick, 0, 0, 0);
	MidiFileEvent_t sustain_event = MidiFileTrack_createControlChangeEvent(track, tick, 0, 64, 0);

	for (int channel_number = 0; channel_number < 16; channel_number++)
	{
		MidiFileNoteOffEvent_setChannel(note_event, channel_number);
		MidiFileControlChangeEvent_setChannel(sustain_event, channel_number);

		for (int note_number = 0; note_number < 127; note_number++)
		{
			MidiFileNoteOffEvent_setNote(note_event, note_number);
			(*(player->visitor_callback))(note_event, player->visitor_callback_user_data);
		}

		(*(player->visitor_callback))(sustain_event, player->visitor_callback_user_data);
	}

	MidiFile_free(midi_file);
}

static void *thread_main(void *arg)
{
	MidiFilePlayer_t player = (MidiFilePlayer_t)(arg);
	player->is_running = 1;
	player->absolute_start_time = MidiFilePlayer_getCurrentTime();

	while (!player->should_shutdown && (player->event != NULL))
	{
		long sleep_time = (MidiFile_getTimeFromTick(player->midi_file, MidiFileEvent_getTick(player->event)) * 1000) - (MidiFilePlayer_getCurrentTime() - (player->absolute_start_time - player->relative_start_time));

		if (sleep_time > 0)
		{
			MidiFilePlayerWaitLock_wait(player->wait_lock, sleep_time);
		}
		else
		{
			(*(player->visitor_callback))(player->event, player->visitor_callback_user_data);
			player->event = MidiFileEvent_getNextEventInFile(player->event);
		}
	}

	player->relative_start_time = MidiFilePlayer_getCurrentTime() - (player->absolute_start_time - player->relative_start_time);
	player->is_running = 0;
	player->should_shutdown = 0;
	stop_held_notes(player);
	return NULL;
}

MidiFilePlayer_t MidiFilePlayer_new(MidiFileEventVisitorCallback_t visitor_callback, void *user_data)
{
	MidiFilePlayer_t player;
	if (visitor_callback == NULL) return NULL;
	player = (MidiFilePlayer_t)(malloc(sizeof (struct MidiFilePlayer)));
	player->visitor_callback = visitor_callback;
	player->visitor_callback_user_data = user_data;
	player->midi_file = NULL;
	player->is_running = 0;
	player->should_shutdown = 0;
	player->wait_lock = MidiFilePlayerWaitLock_new();
	return player;
}

int MidiFilePlayer_free(MidiFilePlayer_t player)
{
	if (player == NULL) return -1;
	MidiFilePlayer_pause(player);
	MidiFilePlayerWaitLock_free(player->wait_lock);
	free(player);
	return 0;
}

int MidiFilePlayer_setMidiFile(MidiFilePlayer_t player, MidiFile_t midi_file)
{
	if (player == NULL) return -1;
	MidiFilePlayer_pause(player);
	player->midi_file = midi_file;
	MidiFilePlayer_setTick(player, 0);
	return 0;
}

MidiFile_t MidiFilePlayer_getMidiFile(MidiFilePlayer_t player)
{
	if (player == NULL) return NULL;
	return player->midi_file;
}

int MidiFilePlayer_play(MidiFilePlayer_t player)
{
	if ((player == NULL) || (player->midi_file == NULL) || player->is_running) return -1;
	MidiFilePlayer_startThread(thread_main, player);
	return 0;
}

int MidiFilePlayer_pause(MidiFilePlayer_t player)
{
	if ((player == NULL) || !player->is_running) return -1;
	player->should_shutdown = 1;
	MidiFilePlayerWaitLock_notify(player->wait_lock);
	return 0;
}

int MidiFilePlayer_isRunning(MidiFilePlayer_t player)
{
	if (player == NULL) return 0;
	return player->is_running;
}

long MidiFilePlayer_getTick(MidiFilePlayer_t player)
{
	if ((player == NULL) || (player->midi_file == NULL)) return -1;

	if (player->is_running)
	{
		return MidiFile_getTickFromTime(player->midi_file, (MidiFilePlayer_getCurrentTime() - (player->absolute_start_time - player->relative_start_time)) / 1000.0);
	}
	else
	{
		return MidiFile_getTickFromTime(player->midi_file, player->relative_start_time / 1000.0);
	}
}

int MidiFilePlayer_setTick(MidiFilePlayer_t player, long tick)
{
	if ((player == NULL) || (player->midi_file == NULL)) return -1;
	stop_held_notes(player);
	player->relative_start_time = MidiFile_getTimeFromTick(player->midi_file, tick) * 1000;

	for (player->event = MidiFile_getFirstEvent(player->midi_file); player->event != NULL; player->event = MidiFileEvent_getNextEventInFile(player->event))
	{
		if (MidiFileEvent_getTick(player->event) >= tick) break;
	}

	player->absolute_start_time = MidiFilePlayer_getCurrentTime();
	MidiFilePlayerWaitLock_notify(player->wait_lock);
	return 0;
}


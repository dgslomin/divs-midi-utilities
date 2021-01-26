
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

struct PlaybackHead
{
	MidiFileEvent_t sequence_event;
	long played_time_msecs;
	int played_channel;
	int played_note;
	int played_velocity;
	long stopped_time_msecs;
};

typedef struct PlaybackHead *PlaybackHead_t;

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static float tempo_bpm = 100;
static MidiFile_t midi_file;
static MidiUtilLock_t lock;
static MidiUtilPointerArray_t echo_on_array;
static MidiUtilPointerArray_t echo_off_array;
static MidiUtilPointerArray_t arp_on_array;
static MidiUtilPointerArray_t arp_off_array;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --tempo <bpm, default 100> ] [ --echo <beat> <note interval> <velocity> | --arp <beat> <duration beats> <note interval> <velocity> | --seq <beat> <duration beats> <note interval> <velocity> ] ... [ --arp-length <beats> ] [ --seq-length <beats, default 4> ]\n", program_name);
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

static void loop_thread_main(void *user_data)
{
	while (1)
	{
		long current_time_msecs;
		int echo_on_number, echo_off_number, arp_on_number, arp_off_number;

		MidiUtilLock_lock(lock);
		current_time_msecs = MidiUtil_getCurrentTimeMsecs();

		for (echo_on_number = 0; echo_on_number < MidiUtilPointerArray_getSize(echo_on_array); echo_on_number++)
		{
			PlaybackHead_t echo_on = (PlaybackHead_t)(MidiUtilPointerArray_get(echo_on_array, echo_on_number));

			while (echo_on->sequence_event != NULL)
			{
				float sequence_event_time_msecs = echo_on->played_time_msecs + (MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(echo_on->sequence_event)) * 60000 / tempo_bpm);

				if (sequence_event_time_msecs <= current_time_msecs)
				{
					if (MidiFileEvent_isNoteStartEvent(echo_on->sequence_event))
					{
						int note = echo_on->played_note + MidiFileNoteStartEvent_getNote(echo_on->sequence_event) - 60;
						int velocity = echo_on->played_velocity * MidiFileNoteStartEvent_getVelocity(echo_on->sequence_event) / 127;
						if (note >= 0 && note < 128) send_note_on(echo_on->played_channel, note, velocity);
					}

					echo_on->sequence_event = MidiFileEvent_getNextEventInTrack(echo_on->sequence_event);
				}
				else
				{
					break;
				}
			}

			if (echo_on->sequence_event == NULL)
			{
				MidiUtilPointerArray_remove(echo_on_array, echo_on_number--);
				free(echo_on);
			}
		}

		for (echo_off_number = 0; echo_off_number < MidiUtilPointerArray_getSize(echo_off_array); echo_off_number++)
		{
			PlaybackHead_t echo_off = (PlaybackHead_t)(MidiUtilPointerArray_get(echo_off_array, echo_off_number));

			while (echo_off->sequence_event != NULL)
			{
				float sequence_event_time_msecs = echo_off->played_time_msecs + (MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(echo_off->sequence_event)) * 60000 / tempo_bpm);

				if (sequence_event_time_msecs <= current_time_msecs)
				{
					/* we only care about "note on" events in the echo sequence, even when deciding what "note offs" to send */
					if (MidiFileEvent_isNoteStartEvent(echo_off->sequence_event))
					{
						int note = echo_off->played_note + MidiFileNoteStartEvent_getNote(echo_off->sequence_event) - 60;
						int velocity = echo_off->played_velocity * MidiFileNoteStartEvent_getVelocity(echo_off->sequence_event) / 127;
						if (note >= 0 && note < 128) send_note_off(echo_off->played_channel, note, velocity);
					}

					echo_off->sequence_event = MidiFileEvent_getNextEventInTrack(echo_off->sequence_event);
				}
				else
				{
					break;
				}
			}

			if (echo_off->sequence_event == NULL)
			{
				MidiUtilPointerArray_remove(echo_off_array, echo_off_number--);
				free(echo_off);
			}
		}

		for (arp_on_number = 0; arp_on_number < MidiUtilPointerArray_getSize(arp_on_array); arp_on_number++)
		{
			PlaybackHead_t arp_on = (PlaybackHead_t)(MidiUtilPointerArray_get(arp_on_array, arp_on_number));

			while (arp_on->sequence_event != NULL)
			{
				float sequence_event_time_msecs = arp_on->played_time_msecs + (MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(arp_on->sequence_event)) * 60000 / tempo_bpm);

				if (sequence_event_time_msecs <= current_time_msecs)
				{
					if (MidiFileEvent_isNoteStartEvent(arp_on->sequence_event))
					{
						int note = arp_on->played_note + MidiFileNoteStartEvent_getNote(arp_on->sequence_event) - 60;
						int velocity = arp_on->played_velocity * MidiFileNoteStartEvent_getVelocity(arp_on->sequence_event) / 127;
						if (note >= 0 && note < 128) send_note_on(arp_on->played_channel, note, velocity);
						arp_on->sequence_event = MidiFileEvent_getNextEventInTrack(arp_on->sequence_event);
					}
					else if (MidiFileEvent_isNoteEndEvent(arp_on->sequence_event))
					{
						int note = arp_on->played_note + MidiFileNoteEndEvent_getNote(arp_on->sequence_event) - 60;
						int velocity = arp_on->played_velocity * MidiFileNoteEndEvent_getVelocity(arp_on->sequence_event) / 127;
						if (note >= 0 && note < 128) send_note_off(arp_on->played_channel, note, velocity);
						arp_on->sequence_event = MidiFileEvent_getNextEventInTrack(arp_on->sequence_event);
					}
					else if (MidiFileEvent_isMarkerEvent(arp_on->sequence_event))
					{
						arp_on->sequence_event = MidiFileTrack_getFirstEvent(MidiFileEvent_getTrack(arp_on->sequence_event));
						arp_on->played_time_msecs = current_time_msecs;
					}
					else
					{
						arp_on->sequence_event = MidiFileEvent_getNextEventInTrack(arp_on->sequence_event);
					}
				}
				else
				{
					break;
				}
			}

			if (arp_on->sequence_event == NULL)
			{
				MidiUtilPointerArray_remove(arp_on_array, arp_on_number--);
				free(arp_on);
			}
		}

		/* play the corresponding "note off" for each "note on" that was active already, but no new "note ons" nor extra "note offs" */
		for (arp_off_number = 0; arp_off_number < MidiUtilPointerArray_getSize(arp_off_array); arp_off_number++)
		{
			PlaybackHead_t arp_off = (PlaybackHead_t)(MidiUtilPointerArray_get(arp_off_array, arp_off_number));

			while (arp_off->sequence_event != NULL)
			{
				float sequence_event_time_msecs = arp_off->played_time_msecs + (MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(arp_off->sequence_event)) * 60000 / tempo_bpm);

				if (sequence_event_time_msecs <= current_time_msecs)
				{
					if (MidiFileEvent_isNoteEndEvent(arp_off->sequence_event))
					{
						MidiFileEvent_t start_event = MidiFileNoteEndEvent_getNoteStartEvent(arp_off->sequence_event);

						if (start_event != NULL)
						{
 							float start_event_time_msecs = arp_off->played_time_msecs + (MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(start_event)) * 60000 / tempo_bpm);

							if (start_event_time_msecs <= arp_off->stopped_time_msecs)
							{
								int note = arp_off->played_note + MidiFileNoteEndEvent_getNote(arp_off->sequence_event) - 60;
								int velocity = arp_off->played_velocity * MidiFileNoteEndEvent_getVelocity(arp_off->sequence_event) / 127;
								if (note >= 0 && note < 128) send_note_off(arp_off->played_channel, note, 0);
							}
						}
					}

					arp_off->sequence_event = MidiFileEvent_getNextEventInTrack(arp_off->sequence_event);
				}
				else
				{
					break;
				}
			}

			if (arp_off->sequence_event == NULL)
			{
				MidiUtilPointerArray_remove(arp_off_array, arp_off_number--);
				free(arp_off);
			}
		}

		MidiUtilLock_unlock(lock);
		MidiUtil_sleep(5); /* tune this for CPU busyness vs responsiveness */
	}
}

static void handle_note_on(int channel, int note, int velocity)
{
	PlaybackHead_t echo_on, arp_on;
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();

	echo_on = (PlaybackHead_t)(malloc(sizeof(struct PlaybackHead)));
	echo_on->sequence_event = MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, 1, 0));
	echo_on->played_time_msecs = current_time_msecs;
	echo_on->played_channel = channel;
	echo_on->played_note = note;
	echo_on->played_velocity = velocity;

	arp_on = (PlaybackHead_t)(malloc(sizeof(struct PlaybackHead)));
	arp_on->sequence_event = MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, 2, 0));
	arp_on->played_time_msecs = current_time_msecs;
	arp_on->played_channel = channel;
	arp_on->played_note = note;
	arp_on->played_velocity = velocity;

	MidiUtilLock_lock(lock);
	MidiUtilPointerArray_add(echo_on_array, echo_on);
	MidiUtilPointerArray_add(arp_on_array, arp_on);
	MidiUtilLock_unlock(lock);
}

static void handle_note_off(int channel, int note, int velocity)
{
	PlaybackHead_t echo_off;
	long current_time_msecs = MidiUtil_getCurrentTimeMsecs();
	int arp_on_number;

	echo_off = (PlaybackHead_t)(malloc(sizeof(struct PlaybackHead)));
	echo_off->sequence_event = MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, 1, 0));
	echo_off->played_time_msecs = current_time_msecs;
	echo_off->played_channel = channel;
	echo_off->played_note = note;
	echo_off->played_velocity = velocity;

	MidiUtilLock_lock(lock);
	MidiUtilPointerArray_add(echo_off_array, echo_off);

	for (arp_on_number = 0; arp_on_number < MidiUtilPointerArray_getSize(arp_on_array); arp_on_number++)
	{
		PlaybackHead_t arp_on = (PlaybackHead_t)(MidiUtilPointerArray_get(arp_on_array, arp_on_number));

		/* turn off the first matching one */
		if ((arp_on->played_channel == channel) && (arp_on->played_note == note))
		{
			MidiUtilPointerArray_remove(arp_on_array, arp_on_number);
			arp_on->stopped_time_msecs = current_time_msecs;
			MidiUtilPointerArray_add(arp_off_array, arp_on);
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

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
	MidiUtilPointerArray_free(echo_on_array);
	MidiUtilPointerArray_free(echo_off_array);
	MidiUtilPointerArray_free(arp_on_array);
	MidiUtilPointerArray_free(arp_off_array);
	MidiUtilLock_free(lock);
	MidiFile_free(midi_file);
}

int main(int argc, char **argv)
{
	int i;
	float arp_length_beats = -1;
	float seq_length_beats = 4;

	midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	lock = MidiUtilLock_new();
	echo_on_array = MidiUtilPointerArray_new(1024);
	echo_off_array = MidiUtilPointerArray_new(1024);
	arp_on_array = MidiUtilPointerArray_new(1024);
	arp_off_array = MidiUtilPointerArray_new(1024);

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
			MidiFileTrack_createNoteOnEvent(MidiFile_getTrackByNumber(midi_file, 1, 1), MidiFile_getTickFromBeat(midi_file, beat), 0, 60 + note_interval, velocity);
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
			MidiFileTrack_createNoteStartAndEndEvents(MidiFile_getTrackByNumber(midi_file, 2, 1), MidiFile_getTickFromBeat(midi_file, beat), MidiFile_getTickFromBeat(midi_file, beat + duration_beats), 0, 60 + note_interval, velocity, 0);
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
			MidiFileTrack_createNoteStartAndEndEvents(MidiFile_getTrackByNumber(midi_file, 3, 1), MidiFile_getTickFromBeat(midi_file, beat), MidiFile_getTickFromBeat(midi_file, beat + duration_beats), 0, 60 + note_interval, velocity, 0);
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
	if (arp_length_beats >= 0) MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(midi_file, 2, 1), MidiFile_getTickFromBeat(midi_file, arp_length_beats), "loop");
	MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(midi_file, 3, 1), MidiFile_getTickFromBeat(midi_file, seq_length_beats), "loop");
	MidiUtil_startThread(loop_thread_main, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


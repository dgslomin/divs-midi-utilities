
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil.h>

int should_shutdown = 0;
MidiUtilLock_t lock = NULL;

static void interrupt_handler(void *arg)
{
	MidiUtilLock_lock(lock);
	should_shutdown = 1;
	MidiUtilLock_notify(lock);
	MidiUtilLock_unlock(lock);
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --out <port> [ --from ( tick:<tick> | beat:<beat> | mb:<m>:<b> | mbt:<m>:<b>:<t> | time:<time> | hms:<h>:<m>:<s> | hmsf:<h>:<m>:<s>:<f> | marker:<marker> ) ] [ --to ( tick:<tick> | beat:<beat> | mb:<m>:<b> | mbt:<m>:<b>:<t> | time:<time> | hms:<h>:<m>:<s> | hmsf:<h>:<m>:<s>:<f> | marker:<marker> ) ] [ ( --solo-track <n> ) ... | ( --mute-track <n> ) ... ] [ --extra-time <seconds> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	char *midi_out_port = NULL;
	char *from_string = NULL;
	char *to_string = NULL;
	int number_of_solo_tracks = 0;
	int solo_tracks[1024];
	int number_of_mute_tracks = 0;
	int mute_tracks[1024];
	float extra_time = 0.0;
	char *filename = NULL;
	MidiFile_t midi_file;
	RtMidiOutPtr midi_out = NULL;
	RtMidiOutPtr track_midi_outs[1024];
	unsigned long start_time = 0;
	MidiFileEvent_t midi_file_event;
	long from_tick = -1;
	long to_tick = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_out_port = argv[i];
		}
		else if (strcmp(argv[i], "--from") == 0)
		{
			if (++i == argc) usage(argv[0]);
			from_string = argv[i];
		}
		else if (strcmp(argv[i], "--to") == 0)
		{
			if (++i == argc) usage(argv[0]);
			to_string = argv[i];
		}
		else if (strcmp(argv[i], "--solo-track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			solo_tracks[number_of_solo_tracks++] = atoi(argv[i]);
			number_of_solo_tracks++;
		}
		else if (strcmp(argv[i], "--mute-track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			mute_tracks[number_of_mute_tracks++] = atoi(argv[i]);
			number_of_mute_tracks++;
		}
		else if (strcmp(argv[i], "--extra-time") == 0)
		{
			if (++i == argc) usage(argv[0]);
			extra_time = (float)(atof(argv[i]));
		}
		else
		{
			filename = argv[i];
		}
	}

	if ((midi_out_port == NULL) || (filename == NULL))
	{
		usage(argv[0]);
	}

	if ((midi_file = MidiFile_load(filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot open \"%s\".\n", filename);
		return 1;
	}

	if (from_string != NULL)
	{
		if (strncmp(from_string, "tick:", 5) == 0)
		{
			from_tick = atol(from_string + 5);
		}
		else if (strncmp(from_string, "beat:", 5) == 0)
		{
			from_tick = MidiFile_getTickFromBeat(midi_file, atof(from_string + 5));
		}
		else if (strncmp(from_string, "mb:", 3) == 0)
		{
			from_tick = MidiFile_getTickFromMeasureBeatString(midi_file, from_string + 3);
		}
		else if (strncmp(from_string, "mbt:", 4) == 0)
		{
			from_tick = MidiFile_getTickFromMeasureBeatTickString(midi_file, from_string + 4);
		}
		else if (strncmp(from_string, "time:", 5) == 0)
		{
			from_tick = MidiFile_getTickFromTime(midi_file, atof(from_string + 5));
		}
		else if (strncmp(from_string, "hms:", 4) == 0)
		{
			from_tick = MidiFile_getTickFromHourMinuteSecondString(midi_file, from_string + 4);
		}
		else if (strncmp(from_string, "hmsf:", 5) == 0)
		{
			from_tick = MidiFile_getTickFromHourMinuteSecondFrameString(midi_file, from_string + 5);
		}
		else if (strncmp(from_string, "marker:", 7) == 0)
		{
			from_tick = MidiFile_getTickFromMarker(midi_file, from_string + 7);
		}
	}

	if (to_string != NULL)
	{
		if (strncmp(to_string, "tick:", 5) == 0)
		{
			to_tick = atol(to_string + 5);
		}
		else if (strncmp(to_string, "beat:", 5) == 0)
		{
			to_tick = MidiFile_getTickFromBeat(midi_file, atof(to_string + 5));
		}
		else if (strncmp(to_string, "mb:", 3) == 0)
		{
			to_tick = MidiFile_getTickFromMeasureBeatString(midi_file, to_string + 3);
		}
		else if (strncmp(to_string, "mbt:", 4) == 0)
		{
			to_tick = MidiFile_getTickFromMeasureBeatTickString(midi_file, to_string + 4);
		}
		else if (strncmp(to_string, "time:", 5) == 0)
		{
			to_tick = MidiFile_getTickFromTime(midi_file, atof(to_string + 5));
		}
		else if (strncmp(to_string, "hms:", 4) == 0)
		{
			to_tick = MidiFile_getTickFromHourMinuteSecondString(midi_file, to_string + 4);
		}
		else if (strncmp(to_string, "hmsf:", 5) == 0)
		{
			to_tick = MidiFile_getTickFromHourMinuteSecondFrameString(midi_file, to_string + 5);
		}
		else if (strncmp(to_string, "marker:", 7) == 0)
		{
			to_tick = MidiFile_getTickFromMarker(midi_file, to_string + 7);
		}
	}

	if ((midi_out = rtmidi_open_out_port("playsmf", midi_out_port, "out")) == NULL)
	{
		printf("Error:  Cannot open MIDI output port \"%s\".\n", midi_out_port);
		return 1;
	}

	{
		int number_of_tracks = MidiFile_getNumberOfTracks(midi_file);
		int track_number;

		if (number_of_solo_tracks > 0)
		{
			int solo_track_number;

			for (track_number = 0; track_number < number_of_tracks; track_number++) track_midi_outs[track_number] = NULL;

			for (solo_track_number = 0; solo_track_number < number_of_solo_tracks; solo_track_number++)
			{
				if (solo_tracks[solo_track_number] < number_of_tracks) track_midi_outs[solo_tracks[solo_track_number]] = midi_out;
			}
		}
		else
		{
			int mute_track_number;

			for (track_number = 0; track_number < number_of_tracks; track_number++) track_midi_outs[track_number] = midi_out;

			for (mute_track_number = 0; mute_track_number < number_of_mute_tracks; mute_track_number++)
			{
				if (mute_tracks[mute_track_number] < number_of_tracks) track_midi_outs[mute_tracks[mute_track_number]] = NULL;
			}
		}
	}

	lock = MidiUtilLock_new();
	MidiUtil_setInterruptHandler(interrupt_handler, NULL);

	for (midi_file_event = MidiFile_getFirstEvent(midi_file); midi_file_event != NULL; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event))
	{
		if (MidiFileEvent_getType(midi_file_event) != MIDI_FILE_EVENT_TYPE_META)
		{
			long tick = MidiFileEvent_getTick(midi_file_event);
			int in_range = (((from_tick < 0) || (tick >= from_tick)) && ((to_tick < 0) || (tick <= to_tick)));
			RtMidiOutPtr midi_out = track_midi_outs[MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event))];

			if ((!should_shutdown) && in_range)
			{
				unsigned long event_time = (unsigned long)(MidiFile_getTimeFromTick(midi_file, tick) * 1000);

				while (!should_shutdown)
				{
					unsigned long current_time = MidiUtil_getCurrentTimeMsecs();

					if (start_time == 0)
					{
						start_time = current_time - event_time; /* fake start time based on range */
					}

					if (current_time - start_time < event_time)
					{
						MidiUtilLock_lock(lock);
						MidiUtilLock_wait(lock, event_time - (current_time - start_time));
						MidiUtilLock_unlock(lock);
					}
					else
					{
						break;
					}
				}
			}

			if (midi_out != NULL)
			{
				if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_SYSEX)
				{
					rtmidi_out_send_message(midi_out, (const unsigned char *)(MidiFileSysexEvent_getData(midi_file_event)), MidiFileSysexEvent_getDataLength(midi_file_event));
				}
				else if ((should_shutdown && !MidiFileEvent_isNoteStartEvent(midi_file_event)) || (!should_shutdown && (in_range || !MidiFileEvent_isNoteEvent(midi_file_event))))
				{
					unsigned long data = MidiFileVoiceEvent_getData(midi_file_event);
					rtmidi_out_send_message(midi_out, (const unsigned char *)(&data), MidiFileVoiceEvent_getDataLength(midi_file_event));
				}
			}
		}
	}

	if ((extra_time > 0) && !should_shutdown)
	{
		MidiUtilLock_lock(lock);
		MidiUtilLock_wait(lock, (long)(extra_time * 1000.0));
		MidiUtilLock_unlock(lock);
	}

	MidiUtil_setInterruptHandler(NULL, NULL);
	MidiUtilLock_free(lock);
	rtmidi_close_port(midi_out);
	MidiFile_free(midi_file);
	return 0;
}


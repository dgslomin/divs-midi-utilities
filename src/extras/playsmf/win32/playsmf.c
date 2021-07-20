
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <midifile.h>

int should_shutdown = 0;
int sysex_message_is_sent;
HANDLE signalling_object;

static BOOL WINAPI interrupt_handler(DWORD control_type)
{
	should_shutdown = 1;
	PulseEvent(signalling_object);
	return TRUE;
}

static void CALLBACK midi_out_handler(HMIDIOUT midi_out, UINT message_type, DWORD user_data, DWORD midi_message, DWORD param2)
{
	sysex_message_is_sent = 1;
	PulseEvent(signalling_object);
}

static int get_midi_out_number(char *name)
{
	int midi_out_number, maximum_midi_out_number;

	for (midi_out_number = 0, maximum_midi_out_number = midiOutGetNumDevs(); midi_out_number < maximum_midi_out_number; midi_out_number++)
	{
		MIDIOUTCAPS midi_out_caps;
		midiOutGetDevCaps(midi_out_number, &midi_out_caps, sizeof(midi_out_caps));
		if (strcmp(midi_out_caps.szPname, name) == 0) return midi_out_number;
	}

	if (sscanf(name, "%d", &midi_out_number) == 1) return midi_out_number;
	return -1;
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --out <device id> ] [ --track-out <n> <device id> ] ... [ --from ( tick:<tick> | beat:<beat> | mb:<m>:<b> | mbt:<m>:<b>:<t> | time:<time> | hms:<h>:<m>:<s> | hmsf:<h>:<m>:<s>:<f> | marker:<marker> ) ] [ --to ( tick:<tick> | beat:<beat> | mb:<m>:<b> | mbt:<m>:<b>:<t> | time:<time> | hms:<h>:<m>:<s> | hmsf:<h>:<m>:<s>:<f> | marker:<marker> ) ] [ ( --solo-track <n> ) ... | ( --mute-track <n> ) ... ] [ --extra-time <seconds> ] [ -- ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int midi_out_number = MIDI_MAPPER;
	int number_of_midi_outs = 0;
	HMIDIOUT *midi_outs = NULL;
	int number_of_tracks = 0;
	HMIDIOUT *track_midi_outs = NULL;
	char *from_string = NULL;
	char *to_string = NULL;
	int number_of_solo_tracks = 0;
	int *solo_tracks = NULL;
	int number_of_mute_tracks = 0;
	int *mute_tracks = NULL;
	float extra_time = 0.0;
	char *filename = NULL;
	int double_dash = 0;
	MidiFile_t midi_file;
	HMIDIOUT midi_mapper_out = NULL;
	unsigned long start_time = 0;
	MidiFileEvent_t midi_file_event;
	long from_tick = -1;
	long to_tick = -1;

	for (i = 1; i < argc; i++)
	{
		if (!double_dash && ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "/help") == 0) || (strcmp(argv[i], "/h") == 0) || (strcmp(argv[i], "/?") == 0)))
		{
			usage(argv[0]);
		}
		else if (!double_dash && (strcmp(argv[i], "--out") == 0))
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out_number = get_midi_out_number(argv[i])) < 0)
			{
				printf("Error:  No such MIDI output device \"%s\".\n", argv[i]);
				return 1;
			}
		}
		else if (!double_dash && (strcmp(argv[i], "--track-out") == 0))
		{
			int track_number;
			int track_midi_out_number;

			if (++i == argc) usage(argv[0]);
			track_number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);

			if ((track_midi_out_number = get_midi_out_number(argv[i])) < 0)
			{
				printf("Error:  No such MIDI output device \"%s\".\n", argv[i]);
				return 1;
			}

			if (track_midi_out_number >= number_of_midi_outs)
			{
				midi_outs = (HMIDIOUT *)(realloc(midi_outs, (track_midi_out_number + 1) * sizeof (HMIDIOUT)));
				memset(midi_outs + (number_of_midi_outs * sizeof (HMIDIOUT)), 0, (track_midi_out_number + 1 - number_of_midi_outs) * sizeof (HMIDIOUT));
				number_of_midi_outs = track_midi_out_number + 1;
			}

			if (midi_outs[track_midi_out_number] == NULL)
			{
				if (midiOutOpen(&(midi_outs[track_midi_out_number]), track_midi_out_number, (DWORD)(midi_out_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
				{
					fprintf(stderr, "Error:  Cannot open MIDI output port #%d.\n", track_midi_out_number);
					return 1;
				}
			}

			if (track_number >= number_of_tracks)
			{
				track_midi_outs = (HMIDIOUT *)(realloc(track_midi_outs, (track_number + 1) * sizeof (HMIDIOUT)));
				memset(track_midi_outs + (number_of_tracks * sizeof (HMIDIOUT)), 0, (track_number + 1 - number_of_tracks) * sizeof (HMIDIOUT));
				number_of_tracks = track_number + 1;
			}

			track_midi_outs[track_number] = midi_outs[track_midi_out_number];
		}
		else if (!double_dash && (strcmp(argv[i], "--from") == 0))
		{
			if (++i == argc) usage(argv[0]);
			from_string = argv[i];
		}
		else if (!double_dash && (strcmp(argv[i], "--to") == 0))
		{
			if (++i == argc) usage(argv[0]);
			to_string = argv[i];
		}
		else if (!double_dash && (strcmp(argv[i], "--solo-track") == 0))
		{
			if (++i == argc) usage(argv[0]);
			solo_tracks = (int *)(realloc(solo_tracks, (number_of_solo_tracks + 1) * sizeof (int)));
			solo_tracks[number_of_solo_tracks] = atoi(argv[i]);
			number_of_solo_tracks++;
		}
		else if (!double_dash && (strcmp(argv[i], "--mute-track") == 0))
		{
			if (++i == argc) usage(argv[0]);
			mute_tracks = (int *)(realloc(mute_tracks, (number_of_mute_tracks + 1) * sizeof (int)));
			mute_tracks[number_of_mute_tracks] = atoi(argv[i]);
			number_of_mute_tracks++;
		}
		else if (!double_dash && (strcmp(argv[i], "--extra-time") == 0))
		{
			if (++i == argc) usage(argv[0]);
			extra_time = (float)(atof(argv[i]));
		}
		else if (!double_dash && (strcmp(argv[i], "--") == 0))
		{
			double_dash = 1;
		}
		else if (filename == NULL)
		{
			filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (filename == NULL)
	{
		usage(argv[0]);
	}

	if ((midi_file = MidiFile_load(filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot open \"%s\".\n", filename);
		return 1;
	}

	{
		int real_number_of_tracks = MidiFile_getNumberOfTracks(midi_file);

		if (number_of_tracks < real_number_of_tracks)
		{
			track_midi_outs = (HMIDIOUT *)(realloc(track_midi_outs, real_number_of_tracks * sizeof (HMIDIOUT)));
			memset(track_midi_outs + (number_of_tracks * sizeof (HMIDIOUT)), 0, (real_number_of_tracks - number_of_tracks) * sizeof (HMIDIOUT));
			number_of_tracks = real_number_of_tracks;
		}
	}

	{
		HMIDIOUT midi_out;
		int track_number;

		if (midi_out_number == MIDI_MAPPER)
		{
			if (midiOutOpen(&midi_mapper_out, MIDI_MAPPER, (DWORD)(midi_out_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
			{
				fprintf(stderr, "Error:  Cannot open default MIDI output port.\n");
				return 1;
			}

			midi_out = midi_mapper_out;
		}
		else
		{
			if (midi_out_number >= number_of_midi_outs)
			{
				midi_outs = (HMIDIOUT *)(realloc(midi_outs, (midi_out_number + 1) * sizeof (HMIDIOUT)));
				memset(midi_outs + (number_of_midi_outs * sizeof (HMIDIOUT)), 0, (midi_out_number + 1 - number_of_midi_outs) * sizeof (HMIDIOUT));
				number_of_midi_outs = midi_out_number + 1;
			}

			if (midi_outs[midi_out_number] == NULL)
			{
				if (midiOutOpen(&(midi_outs[midi_out_number]), midi_out_number, (DWORD)(midi_out_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
				{
					fprintf(stderr, "Error:  Cannot open MIDI output port #%d.\n", midi_out_number);
					return 1;
				}
			}

			midi_out = midi_outs[midi_out_number];
		}

		for (track_number = 0; track_number < number_of_tracks; track_number++)
		{
			if (track_midi_outs[track_number] == NULL)
			{
				track_midi_outs[track_number] = midi_out;
			}
		}
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

	if (number_of_solo_tracks > 0)
	{
		MidiFileTrack_t midi_file_track;
		MidiFileTrack_t next_midi_file_track = NULL;
		int solo_track_number;

		for (midi_file_track = MidiFile_getTrackByNumber(midi_file, 1, 0); midi_file_track != NULL; midi_file_track = next_midi_file_track)
		{
			next_midi_file_track = MidiFileTrack_getNextTrack(midi_file_track);

			for (solo_track_number = 0; solo_track_number < number_of_solo_tracks; solo_track_number++)
			{
				if (solo_tracks[solo_track_number] == MidiFileTrack_getNumber(midi_file_track)) break;
			}

			if (solo_track_number == number_of_solo_tracks) track_midi_outs[MidiFileTrack_getNumber(midi_file_track)] = NULL;
		}
	}
	else if (number_of_mute_tracks > 0)
	{
		MidiFileTrack_t midi_file_track;
		MidiFileTrack_t next_midi_file_track = NULL;
		int mute_track_number;

		for (midi_file_track = MidiFile_getTrackByNumber(midi_file, 1, 0); midi_file_track != NULL; midi_file_track = next_midi_file_track)
		{
			next_midi_file_track = MidiFileTrack_getNextTrack(midi_file_track);

			for (mute_track_number = 0; mute_track_number < number_of_mute_tracks; mute_track_number++)
			{
				if (mute_tracks[mute_track_number] == MidiFileTrack_getNumber(midi_file_track))
				{
					track_midi_outs[MidiFileTrack_getNumber(midi_file_track)] = NULL;
					break;
				}
			}
		}
	}

	signalling_object = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetConsoleCtrlHandler(interrupt_handler, TRUE);

	for (midi_file_event = MidiFile_getFirstEvent(midi_file); midi_file_event != NULL; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event))
	{
		if (MidiFileEvent_getType(midi_file_event) != MIDI_FILE_EVENT_TYPE_META)
		{
			long tick = MidiFileEvent_getTick(midi_file_event);
			int in_range = (((from_tick < 0) || (tick >= from_tick)) && ((to_tick < 0) || (tick <= to_tick)));
			HMIDIOUT midi_out = track_midi_outs[MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event))];

			if ((!should_shutdown) && in_range)
			{
				unsigned long event_time = (unsigned long)(MidiFile_getTimeFromTick(midi_file, tick) * 1000);

				while (!should_shutdown)
				{
					unsigned long current_time = timeGetTime();

					if (start_time == 0)
					{
						start_time = current_time - event_time; /* fake start time based on range */
					}

					if (current_time - start_time < event_time)
					{
						WaitForSingleObject(signalling_object, event_time - (current_time - start_time));
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
					MIDIHDR midi_message_header;
					midi_message_header.lpData = MidiFileSysexEvent_getData(midi_file_event);
					midi_message_header.dwBufferLength = MidiFileSysexEvent_getDataLength(midi_file_event);
					midi_message_header.dwFlags = 0;
					midiOutPrepareHeader(midi_out, &midi_message_header, sizeof(midi_message_header));
					sysex_message_is_sent = 0;
					midiOutLongMsg(midi_out, &midi_message_header, sizeof(midi_message_header));

					while (!sysex_message_is_sent)
					{
						WaitForSingleObject(signalling_object, INFINITE);
					}

					midiOutUnprepareHeader(midi_out, &midi_message_header, sizeof(midi_message_header));
				}
				else if ((should_shutdown && !MidiFileEvent_isNoteStartEvent(midi_file_event)) || (!should_shutdown && (in_range || ((MidiFileEvent_getType(midi_file_event) != MIDI_FILE_EVENT_TYPE_NOTE_ON) && (MidiFileEvent_getType(midi_file_event) != MIDI_FILE_EVENT_TYPE_NOTE_OFF)))))
				{
					midiOutShortMsg(midi_out, (DWORD)(MidiFileVoiceEvent_getData(midi_file_event)));
				}
			}
		}
	}

	if ((extra_time > 0) && !should_shutdown) WaitForSingleObject(signalling_object, (unsigned long)(extra_time * 1000.0));

	SetConsoleCtrlHandler(interrupt_handler, FALSE);
	CloseHandle(signalling_object);

	for (midi_out_number = 0; midi_out_number < number_of_midi_outs; midi_out_number++)
	{
		if (midi_outs[midi_out_number] != NULL) midiOutClose(midi_outs[midi_out_number]);
	}

	if (midi_mapper_out != NULL) midiOutClose(midi_mapper_out);

	MidiFile_free(midi_file);
	if (mute_tracks != NULL) free(mute_tracks);
	if (solo_tracks != NULL) free(solo_tracks);
	if (track_midi_outs != NULL) free(track_midi_outs);
	if (midi_outs != NULL) free(midi_outs);
	return 0;
}


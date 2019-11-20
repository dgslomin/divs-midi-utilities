
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --from <time> ] [ --to <time> ] [ --out <filename.mid> ] <filename.mid>\n", program_name);
	fprintf(stderr, "where <time> is ( tick:<tick> | measure:<measure> | beat:<beat> | mb:<measure>:<beat> | mbt:<measure>:<beat>:<tick> | time:<second> | hms:<hour>:<minute>:<second> | hmsf:<hour>:<minute>:<second>:<frame> | marker:<marker-name> )\n");
	exit(1);
}

static long get_tick_from_time_string(MidiFile_t midi_file, char *time_string)
{
	if (strncmp(time_string, "tick:", 5) == 0) return atol(time_string + 5);
	if (strncmp(time_string, "beat:", 5) == 0) return MidiFile_getTickFromBeat(midi_file, atof(time_string + 5));
	if (strncmp(time_string, "measure:", 8) == 0) return MidiFile_getTickFromMeasure(midi_file, atof(time_string + 8));
	if (strncmp(time_string, "mb:", 3) == 0) return MidiFile_getTickFromMeasureBeatString(midi_file, time_string + 3);
	if (strncmp(time_string, "mbt:", 4) == 0) return MidiFile_getTickFromMeasureBeatTickString(midi_file, time_string + 4);
	if (strncmp(time_string, "time:", 5) == 0) return MidiFile_getTickFromTime(midi_file, atof(time_string + 5));
	if (strncmp(time_string, "hms:", 4) == 0) return MidiFile_getTickFromHourMinuteSecondString(midi_file, time_string + 4);
	if (strncmp(time_string, "hmsf:", 5) == 0) return MidiFile_getTickFromHourMinuteSecondFrameString(midi_file, time_string + 5);
	if (strncmp(time_string, "marker:", 7) == 0) return MidiFile_getTickFromMarker(midi_file, time_string + 7);
	return -1;
}

int main(int argc, char **argv)
{
	char *from_string = NULL;
	char *to_string = NULL;
	char *output_filename = NULL;
	char *input_filename = NULL;
	int i;
	MidiFile_t midi_file;
	long from_tick;
	long to_tick;
	MidiFileEvent_t event;
	MidiFileEvent_t next_event;
	MidiFileTrack_t track;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
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
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			output_filename = argv[i];
		}
		else if (input_filename == NULL)
		{
			input_filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (input_filename == NULL) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	if (from_string == NULL)
	{
		from_tick = 0;
	}
	else
	{
		from_tick = get_tick_from_time_string(midi_file, from_string);
		if (from_tick < 0) usage(argv[0]);
	}

	if (to_string == NULL)
	{
		to_tick = MidiFileEvent_getTick(MidiFile_getLastEvent(midi_file));
	}
	else
	{
		to_tick = get_tick_from_time_string(midi_file, to_string);
		if (to_tick < 0) usage(argv[0]);
	}

	for (event = MidiFile_getFirstEvent(midi_file); event != NULL; event = next_event)
	{
		long tick = MidiFileEvent_getTick(event);
		next_event = MidiFileEvent_getNextEventInFile(event);

		if (tick >= from_tick)
		{
			if (tick < to_tick)
			{
				MidiFileEvent_delete(event);
			}
			else
			{
				MidiFileEvent_setTick(event, tick - to_tick + from_tick);
			}
		}
	}

	for (track = MidiFile_getFirstTrack(midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		long end_tick = MidiFileTrack_getEndTick(track);

		if (end_tick >= from_tick)
		{
			if (end_tick < to_tick)
			{
				MidiFileTrack_setEndTick(track, MidiFileEvent_getTick(MidiFileTrack_getLastEvent(track)));
			}
			else
			{
				MidiFileTrack_setEndTick(track, end_tick - to_tick + from_tick);
			}
		}
	}

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	return 0;
}


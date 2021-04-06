
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --from <time> ] [ --to <time> ] [ --track <n> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *from_string = NULL;
	char *to_string = NULL;
	int track_number = -1;
	char *input_filename = NULL;
	int i;
	MidiFile_t midi_file;
	MidiFileEvent_t event;
	long from_tick;
	long to_tick;
	float average_velocity_numerator = 0.0;
	long average_velocity_denominator = 0;

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
		else if (strcmp(argv[i], "--track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			track_number = atoi(argv[i]);
		}
		else
		{
			input_filename = argv[i];
		}
	}

	if (input_filename == NULL) usage(argv[0]);

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	from_tick = MidiFile_getTickFromTimeString(midi_file, from_string);
	to_tick = MidiFile_getTickFromTimeString(midi_file, to_string);

	for (event = ((track_number < 0) ? MidiFile_getFirstEvent(midi_file) : MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, track_number, 0))); event != NULL; event = ((track_number < 0) ? MidiFileEvent_getNextEventInFile(event) : MidiFileEvent_getNextEventInTrack(event)))
	{
		if (MidiFileEvent_isNoteStartEvent(event))
		{
			long tick = MidiFileEvent_getTick(event);

			if (((from_tick < 0) || (tick >= from_tick)) && ((to_tick < 0) || (tick <= to_tick)))
			{
				average_velocity_numerator += MidiFileNoteStartEvent_getVelocity(event);
				average_velocity_denominator++;
			}
		}
	}

	if (average_velocity_denominator == 0)
	{
		printf("0.0\n");
	}
	else
	{
		printf("%f\n", average_velocity_numerator / average_velocity_denominator);
	}

	MidiFile_free(midi_file);
	return 0;
}


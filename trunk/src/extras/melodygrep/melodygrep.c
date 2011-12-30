
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	printf("Usage: %s [ -v ] [ --window <intervening notes> ] --file <filename> ( <interval> ... )\n", program_name);
	exit(1);
}

static int check(MidiFileEvent_t event, int window, int number_of_intervals, int *intervals)
{
	if (number_of_intervals == 0)
	{
		return 1;
	}
	else
	{
		MidiFileEvent_t lookahead_event;
		int number_of_intervening_notes = 0;

		for (lookahead_event = MidiFileEvent_getNextEventInFile(event); (lookahead_event != NULL) && (number_of_intervening_notes <= window); lookahead_event = MidiFileEvent_getNextEventInFile(lookahead_event))
		{
			if (MidiFileEvent_isNoteStartEvent(lookahead_event))
			{
				if ((MidiFileNoteStartEvent_getNote(lookahead_event) - MidiFileNoteStartEvent_getNote(event) == intervals[0]) && (check(lookahead_event, window, number_of_intervals - 1, &(intervals[1])))) return 1;
				number_of_intervening_notes++;
			}
		}

		return 0;
	}
}

int main(int argc, char **argv)
{
	int i;
	int inverted = 0;
	int window = 0;
	char *filename = NULL;
	int number_of_intervals = 0;
	int intervals[128];
	MidiFile_t midi_file;
	int found = 0;

	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0))
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "-v") == 0)
		{
			inverted = 1;
		}
		else if (strcmp(argv[i], "--window") == 0)
		{
			if (++i == argc) usage(argv[0]);
			window = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--file") == 0)
		{
			if (++i == argc) usage(argv[0]);
			filename = argv[i];
		}
		else
		{
			intervals[number_of_intervals] = atoi(argv[i]);
			number_of_intervals++;
		}
	}

	if ((filename == NULL) || (number_of_intervals == 0)) usage(argv[0]);

	midi_file = MidiFile_load(filename);

	if (midi_file != NULL)
	{
		MidiFileEvent_t event;

		for (event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
		{
			if (MidiFileEvent_isNoteStartEvent(event))
			{
				if (check(event, window, number_of_intervals, intervals))
				{
					found = 1;
					break;
				}
			}
		}

		MidiFile_free(midi_file);
	}

	if (found != inverted)
	{
		printf("%s\n", filename);
		return 0;
	}
	else
	{
		return 1;
	}
}


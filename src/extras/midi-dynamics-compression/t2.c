
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --floor <n> ] [ --pivot <n> ] [ --out <filename.mid> | --suffix <s> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char output_filename[1024];
	char *output_filename_suffix = "-compressed";
	char *input_filename = NULL;
	int floor_ = 0;
	int pivot = 100;
	int i;
	MidiFile_t midi_file;
	MidiFileEvent_t event;

	output_filename[0] = '\0';

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--floor") == 0)
		{
			if (++i == argc) usage(argv[0]);
			floor_ = (int)(atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--pivot") == 0)
		{
			if (++i == argc) usage(argv[0]);
			pivot = (int)(atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			strcpy(output_filename, argv[i]);
		}
		else if (strcmp(argv[i], "--suffix") == 0)
		{
			if (++i == argc) usage(argv[0]);
			output_filename_suffix = argv[i];
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
	if (output_filename[0] == '\0') sprintf(output_filename, "%.*s%s.mid", strlen(input_filename) - 4, input_filename, output_filename_suffix);

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	for (event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isNoteStartEvent(event))
		{
			int velocity = MidiFileNoteStartEvent_getVelocity(event);
			velocity = (((pivot - floor_) * velocity) / pivot) + floor_;
			MidiFileNoteStartEvent_setVelocity(event, velocity);
		}
	}

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	return 0;
}


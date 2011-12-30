
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --threshold <n> ] [ --limit <n> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *input_filename = NULL;
	char output_filename[1024];
	int threshold = 0;
	int limit = 127;
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
		else if (strcmp(argv[i], "--threshold") == 0)
		{
			if (++i == argc) usage(argv[0]);
			threshold = (int)(atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--limit") == 0)
		{
			if (++i == argc) usage(argv[0]);
			limit = (int)(atoi(argv[i]));
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
	sprintf(output_filename, "%.*s-compressed.mid", strlen(input_filename) - 4, input_filename);

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
			if (velocity > threshold) velocity = (((limit - threshold) * (velocity - threshold)) / (127 - threshold)) + threshold;
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


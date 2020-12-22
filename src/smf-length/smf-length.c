
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *input_filename = NULL;
	int i;
	MidiFile_t midi_file;
	MidiFileEvent_t event;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
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

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	printf("%s\n", MidiFile_getHourMinuteSecondStringFromTick(midi_file, MidiFileEvent_getTick(MidiFile_getLastEvent(midi_file))));

	MidiFile_free(midi_file);
	return 0;
}


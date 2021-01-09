
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	printf("Usage: %s <filename>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *filename;
	MidiFile_t midi_file;

	if (argc != 2) usage(argv[0]);
	filename = argv[1];

	if ((midi_file = MidiFile_load(filename)) == NULL)
	{
		printf("Error: Cannot load MIDI file \"%s\".\n", filename);
		return 1;
	}

	MidiFile_save(midi_file, filename);
	MidiFile_free(midi_file);
	return 0;
}


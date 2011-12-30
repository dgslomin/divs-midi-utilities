
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	printf("Usage: %s --needle <filename.mid> --haystack <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	MidiFile_t needle_midi_file;
	MidiFile_t haystack_midi_file;
	MidiFileEvent_t event;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--needle") == 0)
		{
			if (++i == argc) usage(argv[0]);
			needle_midi_file = MidiFile_load(argv[i]);
			if (needle_midi_file == NULL) fprintf(stderr, "Error:  Cannot load \"%s\".\n", argv[i]);
		}
		else if (strcmp(argv[i], "--haystack") == 0)
		{
			if (++i == argc) usage(argv[0]);
			haystack_midi_file = MidiFile_load(argv[i]);
			if (haystack_midi_file == NULL) fprintf(stderr, "Error:  Cannot load \"%s\".\n", argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((needle_midi_file == NULL) || (haystack_midi_file == NULL)) usage(argv[0]);

	for (event = MidiFile_getFirstEvent(haystack_midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isNoteStartEvent(event))
		{
		}
	}

	MidiFile_free(needle_midi_file);
	MidiFile_free(haystack_midi_file);
	return 0;
}

#if 0

challenges:
1.  intervening notes
2.  differing tempos
3.  differing inaccuracy in timing
4.  differing transposition

for each note start event in file:
	(note, event time in seconds)

#endif


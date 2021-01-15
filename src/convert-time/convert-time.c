
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s <filename.mid> <time>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	MidiFile_t midi_file;
	long tick;

	if (argc != 3) usage(argv[0]);

	if ((midi_file = MidiFile_load(argv[1])) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", argv[1]);
		exit(1);
	}

	if ((tick = MidiFile_getTickFromTimeString(midi_file, argv[2])) < 0) usage(argv[0]);

	printf("tick:%ld\n", tick);
	printf("beat:%0.3f\n", MidiFile_getBeatFromTick(midi_file, tick));
	printf("measure:%0.3f\n", MidiFile_getMeasureFromTick(midi_file, tick));
	printf("mb:%s\n", MidiFile_getMeasureBeatStringFromTick(midi_file, tick));
	printf("mbt:%s\n", MidiFile_getMeasureBeatTickStringFromTick(midi_file, tick));
	printf("time:%0.3f\n", MidiFile_getTimeFromTick(midi_file, tick));
	printf("hms:%s\n", MidiFile_getHourMinuteSecondStringFromTick(midi_file, tick));
	printf("hmsf:%s\n", MidiFile_getHourMinuteSecondFrameStringFromTick(midi_file, tick));

	MidiFile_free(midi_file);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s <filename.mid> ( tick:<tick> | beat:<beat> | measure:<measure> | mb:<measure>:<beat> | mbt:<measure>:<beat>:<tick> | time:<second> | hms:<hour>:<minute>:<second> | hmsf:<hour>:<minute>:<second>:<frame> | marker:<marker-name> )\n", program_name);
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
	MidiFile_t midi_file;
	long tick;

	if (argc != 3) usage(argv[0]);

	if ((midi_file = MidiFile_load(argv[1])) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", argv[1]);
		exit(1);
	}

	if ((tick = get_tick_from_time_string(midi_file, argv[2])) < 0) usage(argv[0]);

	printf("tick:%ld\n", tick);
	printf("beat:%0.3f\n", MidiFile_getBeatFromTick(midi_file, tick));
	printf("measure:%0.3f\n", MidiFile_getMeasureFromTick(midi_file, tick));
	printf("mb:%s\n", MidiFile_getMeasureBeatStringFromTick(midi_file, tick));
	printf("mbt:%s\n", MidiFile_getMeasureBeatTickStringFromTick(midi_file, tick));
	printf("time:%0.3f\n", MidiFile_getTimeFromTick(midi_file, tick));
	printf("hms:%s\n", MidiFile_getHourMinuteSecondStringFromTick(midi_file, tick));
	printf("hmsf:%s\n", MidiFile_getHourMinuteSecondFrameStringFromTick(midi_file, tick));

	return 0;
}


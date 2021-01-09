
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --from <time> ] [ --to <time> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *from_string = NULL;
	char *to_string = NULL;
	char *input_filename = NULL;
	int i;
	MidiFile_t midi_file;
	MidiFileEvent_t event;
	float from_time;
	float to_time;
	float previous_time = 0.0;
	float previous_tempo = 120.0;
	float last_time = 0.0;
	float last_tempo = 0.0;
	float average_tempo_numerator = 0.0;
	float average_tempo_denominator = 0.0;

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

	from_time = MidiFile_getTimeFromTick(midi_file, MidiFile_getTickFromTimeString(midi_file, from_string));
	to_time = MidiFile_getTimeFromTick(midi_file, MidiFile_getTickFromTimeString(midi_file, to_string));
	if (to_time <= from_time) to_time = MidiFile_getTimeFromTick(midi_file, MidiFileEvent_getTick(MidiFile_getLastEvent(midi_file)));

	for (event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isTempoEvent(event))
		{
			float time = MidiFile_getTimeFromTick(midi_file, MidiFileEvent_getTick(event));
			float tempo = MidiFileTempoEvent_getTempo(event);

			if ((time >= from_time) && (time <= to_time))
			{
				float previous_duration = time - ((from_time > previous_time) ? from_time : previous_time);
				average_tempo_numerator += (previous_tempo * previous_duration);
				average_tempo_denominator += previous_duration;
				last_time = time;
				last_tempo = tempo;
			}

			previous_time = time;
			previous_tempo = tempo;
		}
	}

	{
		float last_duration = to_time - last_time;
		average_tempo_numerator += (last_tempo * last_duration);
		average_tempo_denominator += last_duration;
	}

	if (average_tempo_denominator == 0.0)
	{
		printf("120.0\n");
	}
	else
	{
		printf("%f\n", average_tempo_numerator / average_tempo_denominator);
	}

	MidiFile_free(midi_file);
	return 0;
}


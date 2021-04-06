
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *output_filename = NULL;
	char *input_filename = NULL;
	int i;
	MidiFile_t midi_file;
	MidiFileEvent_t ahead_event = NULL;
	MidiFileEvent_t event = NULL;
	MidiFileEvent_t behind_event = NULL;
	float ahead_event_tempo = 0.0;
	float event_tempo = 0.0;
	float behind_event_tempo = 0.0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			output_filename = argv[i];
		}
		else
		{
			input_filename = argv[i];
		}
	}

	if (input_filename == NULL) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	for (ahead_event = MidiFile_getFirstEvent(midi_file); ahead_event != NULL; ahead_event = MidiFileEvent_getNextEventInTrack(ahead_event))
	{
		if (MidiFileEvent_isTempoEvent(ahead_event))
		{
			ahead_event_tempo = MidiFileTempoEvent_getTempo(ahead_event);
			if (behind_event != NULL) MidiFileTempoEvent_setTempo(event, (behind_event_tempo + event_tempo + ahead_event_tempo) / 3.0);
			behind_event = event;
			event = ahead_event;
			behind_event_tempo = event_tempo;
			event_tempo = ahead_event_tempo;
		}
	}

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	MidiFile_free(midi_file);
	return 0;
}


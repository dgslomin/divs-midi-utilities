
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --beat-division <division> [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

static void quantize_event(MidiFileEvent_t event, void *user_data)
{
	int beat_division = *((int *)(user_data));

	if (!MidiFileEvent_isNoteEndEvent(event))
	{
		MidiFile_t midi_file = MidiFileTrack_getMidiFile(MidiFileEvent_getTrack(event));
		long original_tick = MidiFileEvent_getTick(event);
		float original_beat = MidiFile_getBeatFromTick(midi_file, original_tick);
		float quantized_beat = roundf(original_beat * beat_division) / beat_division;
		long quantized_tick = MidiFile_getTickFromBeat(midi_file, quantized_beat);
		MidiFileEvent_setTick(event, quantized_tick);

		if (MidiFileEvent_isNoteStartEvent(event))
		{
			MidiFileEvent_t end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			if (end_event != NULL)
			{
				long original_end_tick = MidiFileEvent_getTick(end_event);
				long quantized_end_tick = original_end_tick + quantized_tick - original_tick;
				MidiFileEvent_setTick(end_event, quantized_end_tick);
			}
		}
	}
}

int main(int argc, char **argv)
{
	int beat_division = -1;
	char *output_filename = NULL;
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
		else if (strcmp(argv[i], "--beat-division") == 0)
		{
			if (++i == argc) usage(argv[0]);
			beat_division = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			output_filename = argv[i];
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

	if ((input_filename == NULL) || (beat_division < 1)) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	MidiFile_visitEvents(midi_file, quantize_event, &beat_division);

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	MidiFile_free(midi_file);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --from <time> ] [ --to <time> ] [ --track <n> ] --amount <n> [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	char *from_string = NULL;
	char *to_string = NULL;
	int track_number = -1;
	float amount = 0.0;
	char *output_filename = NULL;
	char *input_filename = NULL;
	int i;
	MidiFile_t midi_file;
	MidiFileEvent_t event;
	long from_tick;
	long to_tick;

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
		else if (strcmp(argv[i], "--track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			track_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--amount") == 0)
		{
			if (++i == argc) usage(argv[0]);
			amount = atof(argv[i]);
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

	if ((input_filename == NULL) || (amount == 0.0)) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	from_tick = MidiFile_getTickFromTimeString(midi_file, from_string);
	if (from_tick < 0) from_tick = 0;
	to_tick = MidiFile_getTickFromTimeString(midi_file, to_string);
	if (to_tick < 0) to_tick = MidiFileEvent_getTick(MidiFile_getLastEvent(midi_file));

	for (event = ((track_number < 0) ? MidiFile_getFirstEvent(midi_file) : MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, track_number, 0))); event != NULL; event = ((track_number < 0) ? MidiFileEvent_getNextEventInFile(event) : MidiFileEvent_getNextEventInTrack(event)))
	{
		if (MidiFileEvent_isNoteStartEvent(event))
		{
			long tick = MidiFileEvent_getTick(event);

			if ((tick >= from_tick) && (tick <= to_tick))
			{
				int velocity = (int)((float)(MidiFileNoteStartEvent_getVelocity(event)) + amount);
				if (velocity > 127) velocity = 127;
				if (velocity < 0) velocity = 0;
				MidiFileNoteStartEvent_setVelocity(event, velocity);
			}
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


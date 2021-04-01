
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --click-to-beat-ratio <clicks> <beats> [ --channel <default 0> [ --note <default 64> ] [ --velocity <default 64> ] [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int ratio_clicks = 0;
	int ratio_beats = 0;
	int channel = 0;
	int note = 64;
	int velocity = 64;
	char *output_filename = NULL;
	char *input_filename = NULL;
	MidiFile_t midi_file;
	long last_tick;
	MidiFileTrack_t click_track;
	MidiFileEvent_t event;
	long click = 1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--click-to-beat-ratio") == 0)
		{
			if (++i == argc) usage(argv[0]);
			ratio_clicks = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			ratio_beats = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			if (++i == argc) usage(argv[0]);
			channel = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--note") == 0)
		{
			if (++i == argc) usage(argv[0]);
			note = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--velocity") == 0)
		{
			if (++i == argc) usage(argv[0]);
			velocity = atoi(argv[i]);
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

	if ((ratio_clicks == 0) || (ratio_beats == 0) || (input_filename == NULL)) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	if (MidiFile_getFileFormat(midi_file) != 1)
	{
		fprintf(stderr, "Error:  Because this algorithm makes use of multiple, simultaneous tracks, it requires MIDI files in format 1.\n");
		exit(1);
	}

	last_tick = MidiFileEvent_getTick(MidiFile_getLastEvent(midi_file));
	click_track = MidiFile_createTrack(midi_file);

	while (1)
	{
		long click_tick = MidiFile_getTickFromBeat(midi_file, (float)(click) * ratio_beats / ratio_clicks);
		long click_end_tick = MidiFile_getTickFromBeat(midi_file, ((float)(click) + 0.5) * ratio_beats / ratio_clicks);
		if (click_tick > last_tick) break;
		MidiFileTrack_createNoteStartAndEndEvents(click_track, click_tick, click_end_tick, channel, note, velocity, 0);
		click++;
	}

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	MidiFile_free(midi_file);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --click-track <n> [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int click_track_number = 0;
	char *output_filename = NULL;
	char *input_filename = NULL;
	MidiFile_t midi_file;
	MidiFileEvent_t click_event;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--click-track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			click_track_number = atoi(argv[i]);
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

	if ((click_track_number == 0) || (input_filename == NULL)) usage(argv[0]);
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

	for (click_event = MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, click_track_number, 0)); click_event != NULL; click_event = MidiFileEvent_getNextEventInTrack(click_event))
	{
		long click_tick = MidiFileEvent_getTick(click_event);
		float click_time = MidiFile_getTimeFromTick(midi_file, click_tick);

		MidiFileEvent_t previous_event = MidiFileEvent_getPreviousEventInFile(click_event);
		long previous_tick = MidiFileEvent_getTick(previous_event);
		float previous_time = MidiFile_getTimeFromTick(midi_file, previous_tick);

		MidiFileEvent_t next_event = MidiFileEvent_getNextEventInFile(click_event);
		long next_tick = MidiFileEvent_getTick(next_event);
		float next_time = MidiFile_getTimeFromTick(midi_file, next_tick);

		MidiFileEvent_t previous_click_event = MidiFileEvent_getPreviousEventInTrack(click_event);
		long previous_click_tick = MidiFileEvent_getTick(previous_click_event);
		float previous_click_time = MidiFile_getTimeFromTick(midi_file, previous_click_tick);

		MidiFileEvent_t next_click_event = MidiFileEvent_getNextEventInTrack(click_event);
		long next_click_tick = MidiFileEvent_getTick(next_click_event);
		float next_click_time = MidiFile_getTimeFromTick(midi_file, next_click_tick);

		/* only consider events that are less than halfway to the surrounding clicks */

		if ((previous_event != NULL) && (previous_click_event != NULL) && (previous_time < ((previous_click_time + click_time) / 2.0))) previous_event = NULL;
		if ((next_event != NULL) && (next_click_event != NULL) && (next_time > ((click_time + next_click_time) / 2.0))) next_event = NULL;

		/* align it with the nearest candidate event */

		if (previous_event == NULL)
		{
			if (next_event == NULL)
			{
				/* nothing nearby; leave it alone */
			}
			else
			{
				MidiFileEvent_setTick(click_event, next_tick);
			}
		}
		else
		{
			if (next_event == NULL)
			{
				MidiFileEvent_setTick(click_event, previous_tick);
			}
			else
			{
				if (click_time - previous_time < next_time - click_time)
				{
					MidiFileEvent_setTick(click_event, previous_tick);
				}
				else
				{
					MidiFileEvent_setTick(click_event, next_tick);
				}
			}
		}
	}

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	return 0;
}


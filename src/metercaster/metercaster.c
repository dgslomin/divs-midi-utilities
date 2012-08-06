
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

char *left_marker_name = "mc1", *old_right_marker_name = "mc2", *new_right_marker_name = "mc3";
int left_cc_number, left_cc_value, old_right_cc_number, old_right_cc_value, new_right_cc_number, new_right_cc_value;
MidiFileEvent_t left_event = NULL, old_right_event = NULL, new_right_event = NULL;
long left_tick, old_right_tick, new_right_tick;

static void usage(char *program_name)
{
	printf("\n");
	printf("Usage: %s [options] <filename>\n", program_name);
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("   --help\n");
	printf("   --left <adjustment point specifier>\n");
	printf("   --old-right <adjustment point specifier>\n");
	printf("   --new-right <adjustment point specifier>\n");
	printf("   --verbose\n");
	printf("\n");
	printf("Adjustment point specifiers:\n");
	printf("\n");
	printf("   marker <name>\n");
	printf("   cc <number> <value>\n");
	printf("\n");
	printf("Defaults:\n");
	printf("\n");
	printf("   --left marker \"mc1\"\n");
	printf("   --old-right marker \"mc2\"\n");
	printf("   --new-right marker \"mc3\"\n");
	printf("\n");
	exit(1);
}

static void find_adjustment_points(MidiFileEvent_t event, void *user_data)
{
	switch (MidiFileEvent_getType(event))
	{
		case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
		{
			int number = MidiFileControlChangeEvent_getNumber(event);
			int value = MidiFileControlChangeEvent_getValue(event);

			if ((left_marker_name == NULL) && (number == left_cc_number) && (value == left_cc_value))
			{
				left_event = event;
			}
			else if ((old_right_marker_name == NULL) && (number == old_right_cc_number) && (value == old_right_cc_value))
			{
				old_right_event = event;
			}
			else if ((new_right_marker_name == NULL) && (number == new_right_cc_number) && (value == new_right_cc_value))
			{
				new_right_event = event;
			}

			break;
		}
		case MIDI_FILE_EVENT_TYPE_META:
		{
			if (MidiFileMetaEvent_getNumber(event) == 0x06)
			{
				int data_length = MidiFileMetaEvent_getDataLength(event);
				unsigned char *data = MidiFileMetaEvent_getData(event);

				if ((left_marker_name != NULL) && (data_length == strlen(left_marker_name)) && (strncmp(left_marker_name, data, data_length) == 0))
				{
					left_event = event;
				}
				else if ((old_right_marker_name != NULL) && (data_length == strlen(old_right_marker_name)) && (strncmp(old_right_marker_name, data, data_length) == 0))
				{
					old_right_event = event;
				}
				else if ((new_right_marker_name != NULL) && (data_length == strlen(new_right_marker_name)) && (strncmp(new_right_marker_name, data, data_length) == 0))
				{
					new_right_event = event;
				}
			}

			break;
		}
	}
}

static void adjust_timestamps(MidiFileEvent_t event, void *user_data)
{
	long tick = MidiFileEvent_getTick(event);

	if (tick >= left_tick)
	{
		if (tick <= old_right_tick)
		{
			MidiFileEvent_setTick(event, left_tick + ((tick - left_tick) * (new_right_tick - left_tick) / (old_right_tick - left_tick)));
		}
		else
		{
			MidiFileEvent_setTick(event, tick - old_right_tick + new_right_tick);
		}
	}
}

int main(int argc, char **argv)
{
	int i, verbose = 0;
	char *filename = NULL;
	MidiFile_t midi_file;
	MidiFileTrack_t track;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--left") == 0)
		{
			if (++i >= argc) usage(argv[0]);

			if (strcmp(argv[i], "marker") == 0)
			{
				if (++i >= argc) usage(argv[0]);
				left_marker_name = argv[i];
			}
			else if (strcmp(argv[i], "cc") == 0)
			{
				if (++i >= argc) usage(argv[0]);
				if (sscanf(argv[i], "%d", &left_cc_number) != 1) usage(argv[0]);
				if (++i >= argc) usage(argv[0]);
				if (sscanf(argv[i], "%d", &left_cc_value) != 1) usage(argv[0]);
				left_marker_name = NULL;
			}
			else
			{
				usage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "--old-right") == 0)
		{
			if (++i >= argc) usage(argv[0]);

			if (strcmp(argv[i], "marker") == 0)
			{
				if (++i >= argc) usage(argv[0]);
				old_right_marker_name = argv[i];
			}
			else if (strcmp(argv[i], "cc") == 0)
			{
				if (++i >= argc) usage(argv[0]);
				if (sscanf(argv[i], "%d", &old_right_cc_number) != 1) usage(argv[0]);
				if (++i >= argc) usage(argv[0]);
				if (sscanf(argv[i], "%d", &old_right_cc_value) != 1) usage(argv[0]);
				old_right_marker_name = NULL;
			}
			else
			{
				usage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "--new-right") == 0)
		{
			if (++i >= argc) usage(argv[0]);

			if (strcmp(argv[i], "marker") == 0)
			{
				if (++i >= argc) usage(argv[0]);
				new_right_marker_name = argv[i];
			}
			else if (strcmp(argv[i], "cc") == 0)
			{
				if (++i >= argc) usage(argv[0]);
				if (sscanf(argv[i], "%d", &new_right_cc_number) != 1) usage(argv[0]);
				if (++i >= argc) usage(argv[0]);
				if (sscanf(argv[i], "%d", &new_right_cc_value) != 1) usage(argv[0]);
				new_right_marker_name = NULL;
			}
			else
			{
				usage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "--verbose") == 0)
		{
			verbose = 1;
		}
		else if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else
		{
			filename = argv[i];
		}
	}

	if (filename == NULL) usage(argv[0]);

	if (verbose)
	{
		if (left_marker_name == NULL)
		{
			printf("Info: Using cc %d %d for left adjustment point.\n", left_cc_number, left_cc_value);
		}
		else
		{
			printf("Info: Using marker \"%s\" for left adjustment point.\n", left_marker_name);
		}

		if (old_right_marker_name == NULL)
		{
			printf("Info: Using cc %d %d for old right adjustment point.\n", old_right_cc_number, old_right_cc_value);
		}
		else
		{
			printf("Info: Using marker \"%s\" for old right adjustment point.\n", old_right_marker_name);
		}

		if (old_right_marker_name == NULL)
		{
			printf("Info: Using cc %d %d for new right adjustment point.\n", new_right_cc_number, new_right_cc_value);
		}
		else
		{
			printf("Info: Using marker \"%s\" for new right adjustment point.\n", new_right_marker_name);
		}
	}

	if ((midi_file = MidiFile_load(filename)) == NULL)
	{
		printf("Error: Cannot load MIDI file \"%s\".\n", filename);
		return 1;
	}

	if (verbose) printf("Info: Loaded MIDI file \"%s\".\n", filename);

	for (track = MidiFile_getFirstTrack(midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		MidiFileTrack_visitEvents(track, find_adjustment_points, NULL);
	}

	if ((left_event == NULL) || (old_right_event == NULL) || (new_right_event == NULL))
	{
		printf("Error: Cannot find adjustment points.\n");
		return 1;
	}

	left_tick = MidiFileEvent_getTick(left_event);
	old_right_tick = MidiFileEvent_getTick(old_right_event);
	new_right_tick = MidiFileEvent_getTick(new_right_event);

	if (verbose)
	{
		printf("Info: Found left adjustment point at tick %ld.\n", left_tick);
		printf("Info: Found old right adjustment point at tick %ld.\n", old_right_tick);
		printf("Info: Found new right adjustment point at tick %ld.\n", new_right_tick);
	}

	for (track = MidiFile_getFirstTrack(midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		MidiFileTrack_setEndTick(track, MidiFileTrack_getEndTick(track) + new_right_tick - old_right_tick);
		MidiFileTrack_visitEvents(track, adjust_timestamps, NULL);
	}

	if (verbose) printf("Info: Adjusted events.\n");

	MidiFileEvent_setTick(left_event, new_right_tick);
	MidiFileEvent_delete(old_right_event);
	MidiFileEvent_delete(new_right_event);

	if (verbose) printf("Info: Reset adjustment points.\n");

	MidiFile_save(midi_file, filename);

	if (verbose) printf("Info: Saved.\n");

	return 0;
}


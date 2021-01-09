
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s [ --track <n> ] [ --low-note-scale <n> ] [ --high-note-scale <n> ] [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int track_number = -1;
	float low_note_scale_factor = 1.0;
	float high_note_scale_factor = 1.0;
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
		else if (strcmp(argv[i], "--track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			track_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--low-note-scale") == 0)
		{
			if (++i == argc) usage(argv[0]);
			low_note_scale_factor = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--high-note-scale") == 0)
		{
			if (++i == argc) usage(argv[0]);
			high_note_scale_factor = atof(argv[i]);
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

	if (input_filename == NULL) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	for (event = ((track_number < 0) ? MidiFile_getFirstEvent(midi_file) : MidiFileTrack_getFirstEvent(MidiFile_getTrackByNumber(midi_file, track_number, 0))); event != NULL; event = ((track_number < 0) ? MidiFileEvent_getNextEventInFile(event) : MidiFileEvent_getNextEventInTrack(event)))
	{
		if (MidiFileEvent_isNoteStartEvent(event))
		{
			int note = MidiFileNoteStartEvent_getNote(event);
			float scale_factor = low_note_scale_factor + ((high_note_scale_factor - low_note_scale_factor) * (float)(note) / 127.0);
			int velocity = (int)((float)(MidiFileNoteStartEvent_getVelocity(event)) * scale_factor);
			if (velocity > 127) velocity = 127;
			if (velocity < 0) velocity = 0;
			MidiFileNoteStartEvent_setVelocity(event, velocity);
		}
	}

	if (MidiFile_save(midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	return 0;
}


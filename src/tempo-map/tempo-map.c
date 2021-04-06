
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <midiutil-common.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s ( --click-track <n> | --constant-tempo <beats per minute> ) [ --click-to-beat-ratio <clicks> <beats> ] [ --note-click-to-beat-ratio <note> <clicks> <beats> ] ... [ --out <filename.mid> ] <filename.mid>\n", program_name);
	exit(1);
}

static MidiFileEvent_t get_first_click_event(MidiFile_t midi_file, int click_track_number)
{
	MidiFileTrack_t click_track = MidiFile_getTrackByNumber(midi_file, click_track_number, 0);
	MidiFileEvent_t click_event = MidiFileTrack_getFirstEvent(click_track);

	while ((click_event != NULL) && !MidiFileEvent_isNoteStartEvent(click_event))
	{
		click_event = MidiFileEvent_getNextEventInTrack(click_event);
	}

	return click_event;
}

static MidiFileEvent_t get_next_click_event(MidiFileEvent_t click_event)
{
	do
	{
		click_event = MidiFileEvent_getNextEventInTrack(click_event);
	}
	while ((click_event != NULL) && !MidiFileEvent_isNoteStartEvent(click_event));

	return click_event;
}

static MidiFileEvent_t clone_event(MidiFileEvent_t input_event, MidiFile_t output_midi_file, long output_event_tick)
{
	switch (MidiFileEvent_getType(input_event))
	{
		case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
		{
			return MidiFileTrack_createNoteOffEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileNoteOffEvent_getChannel(input_event), MidiFileNoteOffEvent_getNote(input_event), MidiFileNoteOffEvent_getVelocity(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_NOTE_ON:
		{
			return MidiFileTrack_createNoteOnEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileNoteOnEvent_getChannel(input_event), MidiFileNoteOnEvent_getNote(input_event), MidiFileNoteOnEvent_getVelocity(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
		{
			return MidiFileTrack_createKeyPressureEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileKeyPressureEvent_getChannel(input_event), MidiFileKeyPressureEvent_getNote(input_event), MidiFileKeyPressureEvent_getAmount(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
		{
			return MidiFileTrack_createControlChangeEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileControlChangeEvent_getChannel(input_event), MidiFileControlChangeEvent_getNumber(input_event), MidiFileControlChangeEvent_getValue(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
		{
			return MidiFileTrack_createProgramChangeEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileProgramChangeEvent_getChannel(input_event), MidiFileProgramChangeEvent_getNumber(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
		{
			return MidiFileTrack_createChannelPressureEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileChannelPressureEvent_getChannel(input_event), MidiFileChannelPressureEvent_getAmount(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
		{
			return MidiFileTrack_createPitchWheelEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFilePitchWheelEvent_getChannel(input_event), MidiFilePitchWheelEvent_getValue(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_SYSEX:
		{
			return MidiFileTrack_createSysexEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileSysexEvent_getDataLength(input_event), MidiFileSysexEvent_getData(input_event));
		}
		case MIDI_FILE_EVENT_TYPE_META:
		{
			return MidiFileTrack_createMetaEvent(MidiFile_getTrackByNumber(output_midi_file, MidiFileTrack_getNumber(MidiFileEvent_getTrack(input_event)), 1), output_event_tick, MidiFileMetaEvent_getNumber(input_event), MidiFileMetaEvent_getDataLength(input_event), MidiFileMetaEvent_getData(input_event));
		}
		default:
		{
			return NULL;
		}
	}
}

int main(int argc, char **argv)
{
	int i;
	int click_track_number = -1;
	float constant_tempo = -1.0;
	int default_ratio_clicks = 1;
	int default_ratio_beats = 1;
	int note_ratio_clicks[128];
	int note_ratio_beats[128];
	char *output_filename = NULL;
	char *input_filename = NULL;
	MidiFile_t input_midi_file;
	MidiFile_t output_midi_file;
	MidiFileEvent_t input_event;

	for (i = 0; i < 128; i++)
	{
		note_ratio_clicks[i] = 0;
		note_ratio_beats[i] = 0;
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--click-track") == 0)
		{
			if (++i == argc) usage(argv[0]);
			click_track_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--constant-tempo") == 0)
		{
			if (++i == argc) usage(argv[0]);
			constant_tempo = atof(argv[i]);
		}
		else if (strcmp(argv[i], "--click-to-beat-ratio") == 0)
		{
			if (++i == argc) usage(argv[0]);
			default_ratio_clicks = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			default_ratio_beats = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--note-click-to-beat-ratio") == 0)
		{
			int note;
			if (++i == argc) usage(argv[0]);
			note = MidiUtil_getNoteNumberFromName(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_ratio_clicks[note] = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_ratio_beats[note] = atoi(argv[i]);
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

	if (((click_track_number < 0) && (constant_tempo < 0.0)) || (input_filename == NULL)) usage(argv[0]);
	if (output_filename == NULL) output_filename = input_filename;

	if ((input_midi_file = MidiFile_load(input_filename)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot read MIDI file \"%s\".\n", input_filename);
		exit(1);
	}

	if (MidiFile_getFileFormat(input_midi_file) != 1)
	{
		fprintf(stderr, "Error:  Because this algorithm makes use of multiple simultaneous tracks, it requires MIDI files in format 1.\n");
		exit(1);
	}

	output_midi_file = MidiFile_new(1, MidiFile_getDivisionType(input_midi_file), MidiFile_getResolution(input_midi_file));

	if (click_track_number >= 0)
	{
		MidiFileEvent_t left_click_event = NULL;
		MidiFileEvent_t right_click_event = get_first_click_event(input_midi_file, click_track_number);
		float output_beat = 0.0;
		float previous_output_tempo = -1.0;

		while ((right_click_event != NULL) && (MidiFileEvent_getTick(right_click_event) == 0))
		{
			left_click_event = right_click_event;
			right_click_event = get_next_click_event(right_click_event);
		}

		do
		{
			float left_click_time = (left_click_event == NULL) ? 0.0 : MidiFile_getTimeFromTick(input_midi_file, MidiFileEvent_getTick(left_click_event));
			float right_click_time = (right_click_event == NULL) ? (left_click_time + 0.5) : MidiFile_getTimeFromTick(input_midi_file, MidiFileEvent_getTick(right_click_event));
			int ratio_clicks = default_ratio_clicks;
			int ratio_beats = default_ratio_beats;
			float output_tempo;

			if (left_click_event != NULL)
			{
				int note = MidiFileNoteStartEvent_getNote(left_click_event);
				if (note_ratio_clicks[note] != 0) ratio_clicks = note_ratio_clicks[note];
				if (note_ratio_beats[note] != 0) ratio_beats = note_ratio_beats[note];
			}

			output_tempo = 60.0 * ratio_beats / ratio_clicks / (right_click_time - left_click_time);

			if (output_tempo != previous_output_tempo)
			{
				MidiFileTrack_createTempoEvent(MidiFile_getTrackByNumber(output_midi_file, 0, 1), MidiFile_getTickFromBeat(output_midi_file, output_beat), output_tempo);
				previous_output_tempo = output_tempo;
			}

			output_beat += (float)(ratio_beats) / ratio_clicks;

			do
			{
				left_click_event = right_click_event;
				right_click_event = get_next_click_event(right_click_event);
			}
			while ((right_click_event != NULL) && (MidiFileEvent_getTick(right_click_event) == MidiFileEvent_getTick(left_click_event)));
		}
		while (right_click_event != NULL);
	}
	else
	{
		MidiFileTrack_createTempoEvent(MidiFile_getTrackByNumber(output_midi_file, 0, 1), 0, constant_tempo * default_ratio_beats / default_ratio_clicks);
	}

	for (input_event = MidiFile_getFirstEvent(input_midi_file); input_event != NULL; input_event = MidiFileEvent_getNextEventInFile(input_event))
	{
		if (!MidiFileEvent_isTempoEvent(input_event))
		{
			clone_event(input_event, output_midi_file, MidiFile_getTickFromTime(output_midi_file, MidiFile_getTimeFromTick(input_midi_file, MidiFileEvent_getTick(input_event))));
		}
	}

	if (MidiFile_save(output_midi_file, output_filename) < 0)
	{
		fprintf(stderr, "Error:  Cannot write MIDI file \"%s\".\n", output_filename);
		exit(1);
	}

	MidiFile_free(output_midi_file);
	return 0;
}


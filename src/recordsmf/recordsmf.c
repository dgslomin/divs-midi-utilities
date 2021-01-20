
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static char *midi_in_port = NULL;
static char *filename = NULL;
static int save_every_msecs = 0;
static MidiUtilAlarm_t alarm;
static RtMidiInPtr midi_in;
static MidiFile_t midi_file;
static MidiFileTrack_t track;
static long start_time_msecs;
static int changed = 1;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> [ --save-every <msecs> ] <filename>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	long tick = MidiFile_getTickFromTime(midi_file, (float)(MidiUtil_getCurrentTimeMsecs() - start_time_msecs) / 1000.0);

	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			MidiFileTrack_createNoteOffEvent(track, tick, MidiUtilNoteOffMessage_getChannel(message), MidiUtilNoteOffMessage_getNote(message), MidiUtilNoteOffMessage_getVelocity(message));
			changed = 1;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			MidiFileTrack_createNoteOnEvent(track, tick, MidiUtilNoteOnMessage_getChannel(message), MidiUtilNoteOnMessage_getNote(message), MidiUtilNoteOnMessage_getVelocity(message));
			changed = 1;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE:
		{
			MidiFileTrack_createKeyPressureEvent(track, tick, MidiUtilKeyPressureMessage_getChannel(message), MidiUtilKeyPressureMessage_getNote(message), MidiUtilKeyPressureMessage_getAmount(message));
			changed = 1;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			MidiFileTrack_createControlChangeEvent(track, tick, MidiUtilControlChangeMessage_getChannel(message), MidiUtilControlChangeMessage_getNumber(message), MidiUtilControlChangeMessage_getValue(message));
			changed = 1;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE:
		{
			MidiFileTrack_createProgramChangeEvent(track, tick, MidiUtilProgramChangeMessage_getChannel(message), MidiUtilProgramChangeMessage_getNumber(message));
			changed = 1;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CHANNEL_PRESSURE:
		{
			MidiFileTrack_createChannelPressureEvent(track, tick, MidiUtilChannelPressureMessage_getChannel(message), MidiUtilChannelPressureMessage_getAmount(message));
			changed = 1;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL:
		{
			MidiFileTrack_createPitchWheelEvent(track, tick, MidiUtilPitchWheelMessage_getChannel(message), MidiUtilPitchWheelMessage_getValue(message));
			changed = 1;
			break;
		}
		default:
		{
			/* ignore everything else */
			break;
		}
	}
}

static void handle_alarm(int cancelled, void *user_data)
{
	if (cancelled || !changed) return;
	MidiFile_save(midi_file, filename);
	changed = 0;
	MidiUtilAlarm_set(alarm, save_every_msecs, handle_alarm, NULL);
}

static void handle_exit(void *user_data)
{
	MidiUtilAlarm_free(alarm);
	rtmidi_close_port(midi_in);

	if (MidiFile_save(midi_file, filename) != 0)
	{
		fprintf(stderr, "Error:  Cannot save \"%s\".\n", filename);
		exit(1);
	}

	MidiFile_free(midi_file);
}

int main(int argc, char **argv)
{
	int i;

	alarm = MidiUtilAlarm_new();

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_in_port = argv[i];
		}
		else if (strcmp(argv[i], "--save-every") == 0)
		{
			if (++i == argc) usage(argv[0]);
			save_every_msecs = atoi(argv[i]);
		}
		else
		{
			filename = argv[i];
		}
	}

	if ((midi_in_port == NULL) || (filename == NULL)) usage(argv[0]);

	midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	track = MidiFile_createTrack(midi_file); /* conductor track */
	MidiFileTrack_createTimeSignatureEvent(track, 0, 4, 4);
	MidiFileTrack_createKeySignatureEvent(track, 0, 0, 0);
	MidiFileTrack_createTempoEvent(track, 0, 100.0);
	track = MidiFile_createTrack(midi_file); /* main track */
	start_time_msecs = MidiUtil_getCurrentTimeMsecs();

	if ((midi_in = rtmidi_open_in_port("recordsmf", midi_in_port, "recordsmf", handle_midi_message, NULL)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", midi_in_port);
		exit(1);
	}

	if (save_every_msecs > 0) MidiUtilAlarm_set(alarm, save_every_msecs, handle_alarm, NULL);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


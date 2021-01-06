
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil.h>

static char *midi_in_port = NULL;
static char *filename = NULL;
static RtMidiInPtr midi_in;
static MidiFile_t midi_file;
static MidiFileTrack_t track;
static long start_time_msecs;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> <filename>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	long tick = MidiFile_getTickFromTime(midi_file, (float)(MidiUtil_getCurrentTimeMsecs() - start_time_msecs) / 1000.0);

	switch (rtmidi_message_get_type(message))
	{
		case RTMIDI_MESSAGE_TYPE_NOTE_OFF:
		{
			MidiFileTrack_createNoteOffEvent(track, tick, rtmidi_note_off_message_get_channel(message), rtmidi_note_off_message_get_note(message), rtmidi_note_off_message_get_velocity(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_NOTE_ON:
		{
			MidiFileTrack_createNoteOnEvent(track, tick, rtmidi_note_on_message_get_channel(message), rtmidi_note_on_message_get_note(message), rtmidi_note_on_message_get_velocity(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_KEY_PRESSURE:
		{
			MidiFileTrack_createKeyPressureEvent(track, tick, rtmidi_key_pressure_message_get_channel(message), rtmidi_key_pressure_message_get_note(message), rtmidi_key_pressure_message_get_amount(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			MidiFileTrack_createControlChangeEvent(track, tick, rtmidi_control_change_message_get_channel(message), rtmidi_control_change_message_get_number(message), rtmidi_control_change_message_get_value(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_PROGRAM_CHANGE:
		{
			MidiFileTrack_createProgramChangeEvent(track, tick, rtmidi_program_change_message_get_channel(message), rtmidi_program_change_message_get_number(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_CHANNEL_PRESSURE:
		{
			MidiFileTrack_createChannelPressureEvent(track, tick, rtmidi_channel_pressure_message_get_channel(message), rtmidi_channel_pressure_message_get_amount(message));
			break;
		}
		case RTMIDI_MESSAGE_TYPE_PITCH_WHEEL:
		{
			MidiFileTrack_createPitchWheelEvent(track, tick, rtmidi_pitch_wheel_message_get_channel(message), rtmidi_pitch_wheel_message_get_value(message));
			break;
		}
		default:
		{
			/* ignore everything else */
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int i;

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

	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);

	if (MidiFile_save(midi_file, filename) != 0)
	{
		fprintf(stderr, "Error:  Cannot save \"%s\".\n", filename);
		exit(1);
	}

	return 0;
}


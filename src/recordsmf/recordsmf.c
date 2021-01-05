
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

	switch (message[0] & 0xF0)
	{
		case 0x80:
		{
			int channel = message[0] & 0x0F;
			int note = message[1];
			int velocity = message[2];
			MidiFileTrack_createNoteOffEvent(track, tick, channel, note, velocity);
			break;
		}
		case 0x90:
		{
			int channel = message[0] & 0x0F;
			int note = message[1];
			int velocity = message[2];
			MidiFileTrack_createNoteOnEvent(track, tick, channel, note, velocity);
			break;
		}
		case 0xA0:
		{
			int channel = message[0] & 0x0F;
			int note = message[1];
			int amount = message[2];
			MidiFileTrack_createKeyPressureEvent(track, tick, channel, note, amount);
			break;
		}
		case 0xB0:
		{
			int channel = message[0] & 0x0F;
			int number = message[1];
			int value = message[2];
			MidiFileTrack_createControlChangeEvent(track, tick, channel, number, value);
			break;
		}
		case 0xC0:
		{
			int channel = message[0] & 0x0F;
			int number = message[1];
			MidiFileTrack_createProgramChangeEvent(track, tick, channel, number);
			break;
		}
		case 0xD0:
		{
			int channel = message[0] & 0x0F;
			int amount = message[1];
			MidiFileTrack_createChannelPressureEvent(track, tick, channel, amount);
			break;
		}
		case 0xE0:
		{
			int channel = message[0] & 0x0F;
			int value = (message[1] << 7) | message[2];
			MidiFileTrack_createPitchWheelEvent(track, tick, channel, value);
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

	{
		unsigned char time_signature[] = { 4, 2, 24, 8 }; /* 4/4 */
		MidiFileTrack_createMetaEvent(track, 0, 0x58, 4, time_signature);
	}

	{
		unsigned char key_signature[] = { 0, 0 }; /* C major */
		MidiFileTrack_createMetaEvent(track, 0, 0x59, 2, key_signature);
	}

	{
		unsigned char tempo[] = { 0x09, 0x27, 0xC0 }; /* 100 BPM */
		MidiFileTrack_createMetaEvent(track, 0, 0x51, 3, tempo);
	}

	track = MidiFile_createTrack(midi_file); /* main track */
	start_time_msecs = MidiUtil_getCurrentTimeMsecs();

	if ((midi_in = rtmidi_open_in_port("recordsmf", midi_in_port, "in", handle_midi_message, NULL)) == NULL)
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


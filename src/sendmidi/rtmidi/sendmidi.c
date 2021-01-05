
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil.h>

void usage(char *progname)
{
	printf("Usage: %s --out <port> ( --note-off <channel> <pitch> <velocity> | --note-on <channel> <pitch> <velocity> | --key-pressure <channel> <pitch> <amount> | --control-change <channel> <number> <value> | --program-change <channel> <number> | --channel-pressure <channel> <amount> | --pitch-wheel <channel> <amount> )\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	RtMidiOutPtr midi_out = NULL;
	int message_size = 0;
	unsigned char message[3];
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("sendmidi", argv[i], "out")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--note-off") == 0)
		{
			int channel, pitch, velocity;
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &pitch) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &velocity) != 1) usage(argv[0]);
			message_size = 3;
			message[0] = 0x80 | channel;
			message[1] = pitch;
			message[2] = velocity;
		}
		else if (strcmp(argv[i], "--note-on") == 0)
		{
			int channel, pitch, velocity;
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &pitch) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &velocity) != 1) usage(argv[0]);
			message_size = 3;
			message[0] = 0x90 | channel;
			message[1] = pitch;
			message[2] = velocity;
		}
		else if (strcmp(argv[i], "--key-pressure") == 0)
		{
			int channel, pitch, amount;

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &pitch) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &amount) != 1) usage(argv[0]);
			message_size = 3;
			message[0] = 0xA0 | channel;
			message[1] = pitch;
			message[2] = amount;
		}
		else if (strcmp(argv[i], "--control-change") == 0)
		{
			int channel, number, value;
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &number) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &value) != 1) usage(argv[0]);
			message_size = 3;
			message[0] = 0xB0 | channel;
			message[1] = number;
			message[2] = value;
		}
		else if (strcmp(argv[i], "--program-change") == 0)
		{
			int channel, number;
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &number) != 1) usage(argv[0]);
			message_size = 2;
			message[0] = 0xC0 | channel;
			message[1] = number;
		}
		else if (strcmp(argv[i], "--channel-pressure") == 0)
		{
			int channel, amount;

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &amount) != 1) usage(argv[0]);
			message_size = 2;
			message[0] = 0xD0 | channel;
			message[1] = amount;
		}
		else if (strcmp(argv[i], "--pitch-wheel") == 0)
		{
			int channel, amount;

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &amount) != 1) usage(argv[0]);
			message_size = 3;
			message[0] = 0xE0 | channel;
			message[1] = amount & 0x7F;
			message[2] = amount >> 7;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_out == NULL) || (message_size == 0)) usage(argv[0]);
	rtmidi_out_send_message(midi_out, message, message_size);
	MidiUtil_sleep(500); /* make sure the message has time to go out before closing the port */
	rtmidi_close_port(midi_out);
	return 0;
}


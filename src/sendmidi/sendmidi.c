
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

void usage(char *progname)
{
	printf("Usage: %s --out <port> ( --note-off <channel> <note> <velocity> | --note-on <channel> <note> <velocity> | --key-pressure <channel> <note> <amount> | --control-change <channel> <number> <value> | --program-change <channel> <number> | --channel-pressure <channel> <amount> | --pitch-wheel <channel> <value> )\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	RtMidiOutPtr midi_out = NULL;
	unsigned char message[MIDI_UTIL_MESSAGE_SIZE_SHORT_MESSAGE];
	int i;

	message[0] = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("sendmidi", argv[i], "sendmidi")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--note-off") == 0)
		{
			int channel, note, velocity;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			note = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			velocity = atoi(argv[i]);
			MidiUtilMessage_setNoteOff(message, channel, note, velocity);
		}
		else if (strcmp(argv[i], "--note-on") == 0)
		{
			int channel, note, velocity;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			note = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			velocity = atoi(argv[i]);
			MidiUtilMessage_setNoteOn(message, channel, note, velocity);
		}
		else if (strcmp(argv[i], "--key-pressure") == 0)
		{
			int channel, note, amount;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			note = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			amount = atoi(argv[i]);
			MidiUtilMessage_setKeyPressure(message, channel, note, amount);
		}
		else if (strcmp(argv[i], "--control-change") == 0)
		{
			int channel, number, value;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			value = atoi(argv[i]);
			MidiUtilMessage_setControlChange(message, channel, number, value);
		}
		else if (strcmp(argv[i], "--program-change") == 0)
		{
			int channel, number;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			number = atoi(argv[i]);
			MidiUtilMessage_setProgramChange(message, channel, number);
		}
		else if (strcmp(argv[i], "--channel-pressure") == 0)
		{
			int channel, amount;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			amount = atoi(argv[i]);
			MidiUtilMessage_setChannelPressure(message, channel, amount);
		}
		else if (strcmp(argv[i], "--pitch-wheel") == 0)
		{
			int channel, value;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			value = atoi(argv[i]);
			MidiUtilMessage_setPitchWheel(message, channel, value);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_out == NULL) || (message[0] == 0)) usage(argv[0]);
	rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));
	MidiUtil_sleep(500); /* make sure the message has time to go out before closing the port */
	rtmidi_close_port(midi_out);
	return 0;
}


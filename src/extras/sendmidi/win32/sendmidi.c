
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void usage(char *progname)
{
	printf("Usage: %s [ --out <port> ] ( --note-off <channel> <pitch> <velocity> | --note-on <channel> <pitch> <velocity> | --key-pressure <channel> <pitch> <amount> | --control-change <channel> <number> <value> | --program-change <channel> <number> | --channel-pressure <channel> <amount> | --pitch-wheel <channel> <amount> )\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	int i;
	int midi_out_number = MIDI_MAPPER;
	HMIDIOUT midi_out;

	u.bData[0] = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &midi_out_number) != 1) usage(argv[0]);
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

			u.bData[0] = 0x80 | channel;
			u.bData[1] = pitch;
			u.bData[2] = velocity;
			u.bData[3] = 0x00;
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

			u.bData[0] = 0x90 | channel;
			u.bData[1] = pitch;
			u.bData[2] = velocity;
			u.bData[3] = 0x00;
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

			u.bData[0] = 0xA0 | channel;
			u.bData[1] = pitch;
			u.bData[2] = amount;
			u.bData[3] = 0x00;
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

			u.bData[0] = 0xB0 | channel;
			u.bData[1] = number;
			u.bData[2] = value;
			u.bData[3] = 0x00;
		}
		else if (strcmp(argv[i], "--program-change") == 0)
		{
			int channel, number;

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &number) != 1) usage(argv[0]);

			u.bData[0] = 0xC0 | channel;
			u.bData[1] = number;
			u.bData[2] = 0x00;
			u.bData[3] = 0x00;
		}
		else if (strcmp(argv[i], "--channel-pressure") == 0)
		{
			int channel, amount;

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &amount) != 1) usage(argv[0]);

			u.bData[0] = 0xD0 | channel;
			u.bData[1] = amount;
			u.bData[2] = 0x00;
			u.bData[3] = 0x00;
		}
		else if (strcmp(argv[i], "--pitch-wheel") == 0)
		{
			int channel, amount;

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &channel) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &amount) != 1) usage(argv[0]);

			u.bData[0] = 0xE0 | channel;
			u.bData[1] = amount & 0x7F;
			u.bData[2] = amount >> 7;
			u.bData[3] = 0x00;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (u.bData[0] == 0x00) usage(argv[0]);

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	midiOutShortMsg(midi_out, u.dwData);
	Sleep(500); /* make sure the message has time to go out before closing the port */
	midiOutClose(midi_out);

	return 0;
}


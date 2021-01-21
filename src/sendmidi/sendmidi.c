
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

typedef enum
{
	ACTION_NONE,
	ACTION_SINGLE_MESSAGE,
	ACTION_PANIC
}
Action_t;

void usage(char *progname)
{
	printf("Usage: %s --out <port> ( --note-off <channel> <note> <velocity> | --note-on <channel> <note> <velocity> | --key-pressure <channel> <note> <amount> | --control-change <channel> <number> <value> | --program-change <channel> <number> | --channel-pressure <channel> <amount> | --pitch-wheel <channel> <value> | --panic )\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	RtMidiOutPtr midi_out = NULL;
	Action_t action = ACTION_NONE;
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
			note = MidiUtil_getNoteNumberFromName(argv[i]);
			if (++i >= argc) usage(argv[0]);
			velocity = atoi(argv[i]);
			action = ACTION_SINGLE_MESSAGE;
			MidiUtilMessage_setNoteOff(message, channel, note, velocity);
		}
		else if (strcmp(argv[i], "--note-on") == 0)
		{
			int channel, note, velocity;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			note = MidiUtil_getNoteNumberFromName(argv[i]);
			if (++i >= argc) usage(argv[0]);
			velocity = atoi(argv[i]);
			action = ACTION_SINGLE_MESSAGE;
			MidiUtilMessage_setNoteOn(message, channel, note, velocity);
		}
		else if (strcmp(argv[i], "--key-pressure") == 0)
		{
			int channel, note, amount;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			note = MidiUtil_getNoteNumberFromName(argv[i]);
			if (++i >= argc) usage(argv[0]);
			amount = atoi(argv[i]);
			action = ACTION_SINGLE_MESSAGE;
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
			action = ACTION_SINGLE_MESSAGE;
			MidiUtilMessage_setControlChange(message, channel, number, value);
		}
		else if (strcmp(argv[i], "--program-change") == 0)
		{
			int channel, number;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			number = atoi(argv[i]);
			action = ACTION_SINGLE_MESSAGE;
			MidiUtilMessage_setProgramChange(message, channel, number);
		}
		else if (strcmp(argv[i], "--channel-pressure") == 0)
		{
			int channel, amount;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			amount = atoi(argv[i]);
			action = ACTION_SINGLE_MESSAGE;
			MidiUtilMessage_setChannelPressure(message, channel, amount);
		}
		else if (strcmp(argv[i], "--pitch-wheel") == 0)
		{
			int channel, value;
			if (++i >= argc) usage(argv[0]);
			channel = atoi(argv[i]);
			if (++i >= argc) usage(argv[0]);
			value = atoi(argv[i]);
			action = ACTION_SINGLE_MESSAGE;
			MidiUtilMessage_setPitchWheel(message, channel, value);
		}
		else if (strcmp(argv[i], "--panic") == 0)
		{
			action = ACTION_PANIC;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_out == NULL) usage(argv[0]);

	switch (action)
	{
		case ACTION_NONE:
		{
			usage(argv[0]);
			break;
		}
		case ACTION_SINGLE_MESSAGE:
		{
			rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));
			break;
		}
		case ACTION_PANIC:
		{
			int channel, note;

			for (channel = 0; channel < 16; channel++)
			{
				/* all sound off */
				MidiUtilMessage_setControlChange(message, channel, 120, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* all controllers off */
				MidiUtilMessage_setControlChange(message, channel, 121, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* and redundantly in case those aren't supported */
				for (note = 0; note < 128; note++)
				{
					MidiUtilMessage_setNoteOff(message, channel, note, 0);
					rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

					MidiUtilMessage_setKeyPressure(message, channel, note, 0);
					rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));
				}

				/* mod wheel */
				MidiUtilMessage_setControlChange(message, channel, 1, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* expression */
				MidiUtilMessage_setControlChange(message, channel, 11, 127);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* hold */
				MidiUtilMessage_setControlChange(message, channel, 64, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* portamento */
				MidiUtilMessage_setControlChange(message, channel, 65, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* sustenuto */
				MidiUtilMessage_setControlChange(message, channel, 66, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* soft */
				MidiUtilMessage_setControlChange(message, channel, 67, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* legato */
				MidiUtilMessage_setControlChange(message, channel, 68, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* hold 2 */
				MidiUtilMessage_setControlChange(message, channel, 69, 0);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* volume */
				MidiUtilMessage_setControlChange(message, channel, 7, 100);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				/* pan */
				MidiUtilMessage_setControlChange(message, channel, 10, 64);
				rtmidi_out_send_message(midi_out, message, MidiUtilMessage_getSize(message));

				MidiUtilMessage_setPitchWheel(message, channel, 0x2000);
				MidiUtilMessage_setChannelPressure(message, channel, 0);
			}

			break;
		}
	}

	MidiUtil_sleep(500); /* make sure the message has time to go out before closing the port */
	rtmidi_close_port(midi_out);
	return 0;
}


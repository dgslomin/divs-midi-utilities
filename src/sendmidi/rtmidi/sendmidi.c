
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil.h>

void usage(char *progname)
{
	printf("Usage: %s --out <port> ( --note-off <channel> <note> <velocity> | --note-on <channel> <note> <velocity> | --key-pressure <channel> <note> <amount> | --control-change <channel> <number> <value> | --program-change <channel> <number> | --channel-pressure <channel> <amount> | --pitch-wheel <channel> <value> )\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	RtMidiOutPtr midi_out = NULL;
	int message_size = 0;
	unsigned char message[4];
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
			message_size = RTMIDI_MESSAGE_SIZE_NOTE_OFF;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_NOTE_OFF);
			if (++i >= argc) usage(argv[0]);
			rtmidi_note_off_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_note_off_message_set_note(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_note_off_message_set_velocity(message, atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--note-on") == 0)
		{
			message_size = RTMIDI_MESSAGE_SIZE_NOTE_ON;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_NOTE_ON);
			if (++i >= argc) usage(argv[0]);
			rtmidi_note_on_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_note_on_message_set_note(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_note_on_message_set_velocity(message, atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--key-pressure") == 0)
		{
			message_size = RTMIDI_MESSAGE_SIZE_KEY_PRESSURE;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_KEY_PRESSURE);
			if (++i >= argc) usage(argv[0]);
			rtmidi_key_pressure_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_key_pressure_message_set_note(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_key_pressure_message_set_amount(message, atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--control-change") == 0)
		{
			message_size = RTMIDI_MESSAGE_SIZE_CONTROL_CHANGE;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_CONTROL_CHANGE);
			if (++i >= argc) usage(argv[0]);
			rtmidi_control_change_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_control_change_message_set_number(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_control_change_message_set_value(message, atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--program-change") == 0)
		{
			message_size = RTMIDI_MESSAGE_SIZE_PROGRAM_CHANGE;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_PROGRAM_CHANGE);
			if (++i >= argc) usage(argv[0]);
			rtmidi_program_change_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_program_change_message_set_number(message, atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--channel-pressure") == 0)
		{
			message_size = RTMIDI_MESSAGE_SIZE_CHANNEL_PRESSURE;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_CHANNEL_PRESSURE);
			if (++i >= argc) usage(argv[0]);
			rtmidi_channel_pressure_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_channel_pressure_message_set_amount(message, atoi(argv[i]));
		}
		else if (strcmp(argv[i], "--pitch-wheel") == 0)
		{
			message_size = RTMIDI_MESSAGE_SIZE_PITCH_WHEEL;
			rtmidi_message_set_type(message, RTMIDI_MESSAGE_TYPE_PITCH_WHEEL);
			if (++i >= argc) usage(argv[0]);
			rtmidi_pitch_wheel_message_set_channel(message, atoi(argv[i]));
			if (++i >= argc) usage(argv[0]);
			rtmidi_pitch_wheel_message_set_value(message, atoi(argv[i]));
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


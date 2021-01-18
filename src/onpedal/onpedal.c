
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

static RtMidiInPtr midi_in = NULL;
static char *command = NULL;
static int hold_length_msecs = 500;
static char *hold_command = NULL;
static MidiUtilAlarm_t alarm = NULL;
static int is_pressed = 0;
static int alarm_rang = 0;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --command <str> [ --hold-length <msecs> ] [ --hold-command <str> ]\n", program_name);
	exit(1);
}

static void handle_alarm(int cancelled, void *user_data)
{
	if (cancelled) return;
	alarm_rang = 1;
	system(hold_command);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	if ((MidiUtilMessage_getType(message) == MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE) && (MidiUtilControlChangeMessage_getNumber(message) == 64))
	{
		if (MidiUtilControlChangeMessage_getValue(message) >= 64)
		{
			if (!is_pressed)
			{
				is_pressed = 1;

				if (hold_command != NULL)
				{
					alarm_rang = 0;
					MidiUtilAlarm_set(alarm, hold_length_msecs, handle_alarm, NULL);
				}
				else
				{
					system(command);
				}
			}
		}
		else
		{
			if (is_pressed)
			{
				is_pressed = 0;

				if ((hold_command != NULL) && !alarm_rang)
				{
					MidiUtilAlarm_cancel(alarm);
					system(command);
				}
			}
		}
	}
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	MidiUtilAlarm_free(alarm);
}

int main(int argc, char **argv)
{
	int i;
	alarm = MidiUtilAlarm_new();

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("onpedal", argv[i], "onpedal", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--command") == 0)
		{
			if (++i == argc) usage(argv[0]);
			command = argv[i];
		}
		else if (strcmp(argv[i], "--hold-length") == 0)
		{
			if (++i == argc) usage(argv[0]);
			hold_length_msecs = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--hold-command") == 0)
		{
			if (++i == argc) usage(argv[0]);
			hold_command = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (command == NULL)) usage(argv[0]);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


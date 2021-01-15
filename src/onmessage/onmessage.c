
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

RtMidiInPtr midi_in = NULL;
char *note_commands[128];
char *note_down_commands[128];
char *note_up_commands[128];
char *controller_commands[128];
char *controller_down_commands[128];
char *controller_up_commands[128];
char *controller_increase_commands[128];
char *controller_decrease_commands[128];
int previous_controller_value[128];
char *program_commands[128];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> ( --note-command <number> <command> | --note-down-command <number> <command> | --note-up-command <number> <command> | --controller-command <number> <command> | --controller-down-command <number> <command> | --controller-up-command <number> <command> | --controller-increase-command <number> <command> | --controller-decrease-command <number> <command> | --program-command <number> <command> ) ...\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			int note = MidiUtilNoteOffMessage_getNote(message);
			if (note_commands[note] != NULL) system(note_commands[note]);
			if (note_up_commands[note] != NULL) system(note_up_commands[note]);
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int note = MidiUtilNoteOnMessage_getNote(message);
			int velocity = MidiUtilNoteOnMessage_getVelocity(message);
			if (note_commands[note] != NULL) system(note_commands[note]);

			if (velocity > 0)
			{
				if (note_down_commands[note] != NULL) system(note_down_commands[note]);
			}
			else
			{
				if (note_up_commands[note] != NULL) system(note_up_commands[note]);
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			int number = MidiUtilControlChangeMessage_getNumber(message);
			int value = MidiUtilControlChangeMessage_getValue(message);
			int previous_value = previous_controller_value[number];
			if (controller_commands[number] != NULL) system(controller_commands[number]);
			if ((value >= 64) && (previous_value < 64) && (controller_down_commands[number] != NULL)) system(controller_down_commands[number]);
			if ((value < 64) && (previous_value >= 64) && (controller_up_commands[number] != NULL)) system(controller_up_commands[number]);

			if (previous_value >= 0)
			{
				if ((value > previous_value) && (controller_increase_commands[number] != NULL)) system(controller_increase_commands[number]);
				if ((value < previous_value) && (controller_decrease_commands[number] != NULL)) system(controller_decrease_commands[number]);
			}

			previous_controller_value[number] = value;
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PROGRAM_CHANGE:
		{
			int number = MidiUtilProgramChangeMessage_getNumber(message);
			if (program_commands[number] != NULL) system(program_commands[number]);
			break;
		}
		default:
		{
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int i;

	for (i = 0; i < 128; i++)
	{
		note_commands[i] = NULL;
		note_down_commands[i] = NULL;
		note_up_commands[i] = NULL;
		controller_commands[i] = NULL;
		controller_down_commands[i] = NULL;
		controller_up_commands[i] = NULL;
		controller_increase_commands[i] = NULL;
		controller_decrease_commands[i] = NULL;
		previous_controller_value[i] = -1;
		program_commands[i] = NULL;
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("onmessage", argv[i], "onmessage", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--note-command") == 0)
		{
			int note;
			if (++i == argc) usage(argv[0]);
			note = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_commands[note] = argv[i];
		}
		else if (strcmp(argv[i], "--note-down-command") == 0)
		{
			int note;
			if (++i == argc) usage(argv[0]);
			note = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_down_commands[note] = argv[i];
		}
		else if (strcmp(argv[i], "--note-up-command") == 0)
		{
			int note;
			if (++i == argc) usage(argv[0]);
			note = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_up_commands[note] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-down-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_down_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-up-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_up_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-increase-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_increase_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-decrease-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_decrease_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--program-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			program_commands[number] = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_in == NULL) usage(argv[0]);
	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>

struct ControllerHoldId
{
	int number;
};

typedef struct ControllerHoldId *ControllerHoldId_t;

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int hold_length_msecs = 500;
static char *note_commands[128];
static char *controller_commands[128];
static char *controller_hold_commands[128];
static char *pitch_wheel_up_command = NULL;
static char *pitch_wheel_down_command = NULL;
static MidiUtilAlarm_t alarm = NULL;
static int controller_state[128];
static ControllerHoldId_t controller_hold_ids[128];
static int pitch_wheel_state = 0;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> [ --out <port> ] [ --hold-length <msecs> ] [ --note-command <note> <command> | --controller-command <controller number> <command> | --controller-hold-command <controller number> <command> | --pitch-wheel-up-command <command> | --pitch-wheel-down-command <command> ] ...\n", program_name);
	exit(1);
}

/* Call system() in a background thread to prevent it from blocking. */
static void run_command_thread_main(void *user_data)
{
	char *command = (char *)(user_data);
	system(command);
}

static void handle_controller_alarm(int cancelled, void *user_data)
{
	ControllerHoldId_t controller_hold_id = (ControllerHoldId_t)(user_data);

	if (!cancelled)
	{
		if (controller_hold_ids[controller_hold_id->number] == controller_hold_id)
		{
			controller_hold_ids[controller_hold_id->number] = NULL;
			MidiUtil_startThread(run_command_thread_main, controller_hold_commands[controller_hold_id->number]);
		}
	}

	free(controller_hold_id);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			int note = MidiUtilNoteOnMessage_getNote(message);

			if (note_commands[note] == NULL)
			{
				if (midi_out != NULL) rtmidi_out_send_message(midi_out, message, message_size);
			}
			else
			{
				if (MidiUtilNoteOnMessage_getVelocity(message) > 0) MidiUtil_startThread(run_command_thread_main, note_commands[note]);
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_CONTROL_CHANGE:
		{
			int number = MidiUtilControlChangeMessage_getNumber(message);

			if ((controller_commands[number] == NULL) && (controller_hold_commands[number] == NULL))
			{
				if (midi_out != NULL) rtmidi_out_send_message(midi_out, message, message_size);
			}
			else
			{
				int value = MidiUtilControlChangeMessage_getValue(message);

				if ((value >= 72) && (controller_state[number] != 1))
				{
					controller_state[number] = 1;

					if (controller_hold_commands[number] == NULL)
					{
						MidiUtil_startThread(run_command_thread_main, controller_commands[number]);
					}
					else
					{
						/* We can't cancel individual alarms, so let the alarm fire but compare with a unique id to see if it's still relevant. */
						ControllerHoldId_t controller_hold_id = (ControllerHoldId_t)(malloc(sizeof (struct ControllerHoldId)));
						controller_hold_id->number = number;
						controller_hold_ids[number] = controller_hold_id;
						MidiUtilAlarm_add(alarm, hold_length_msecs, handle_controller_alarm, controller_hold_id);
					}
				}
				else if ((value < 56) && (controller_state[number] != 0))
				{
					controller_state[number] = 0;

					if (controller_hold_commands[number] != NULL)
					{
						if (controller_hold_ids[number] != NULL)
						{
							controller_hold_ids[number] = NULL;
							MidiUtil_startThread(run_command_thread_main, controller_commands[number]);
						}
					}
				}
			}

			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_PITCH_WHEEL:
		{
			if ((pitch_wheel_up_command == NULL) && (pitch_wheel_down_command == NULL))
			{
				if (midi_out != NULL) rtmidi_out_send_message(midi_out, message, message_size);
			}
			else
			{
				int value = MidiUtilPitchWheelMessage_getValue(message);

				if (value >= 0x3500)
				{
					if (pitch_wheel_state != 1)
					{
						pitch_wheel_state = 1;
						if (pitch_wheel_up_command != NULL) MidiUtil_startThread(run_command_thread_main, pitch_wheel_up_command);
					}
				}
				else if (value < 0x500)
				{
					if (pitch_wheel_state != -1)
					{
						pitch_wheel_state = -1;
						if (pitch_wheel_down_command != NULL) MidiUtil_startThread(run_command_thread_main, pitch_wheel_down_command);
					}
				}
				else
				{
					pitch_wheel_state = 0;
				}
			}

			break;
		}
		default:
		{
			if (midi_out != NULL) rtmidi_out_send_message(midi_out, message, message_size);
			break;
		}
	}
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	if (midi_out != NULL) rtmidi_close_port(midi_out);
	MidiUtilAlarm_free(alarm);
}

int main(int argc, char **argv)
{
	int i;
	alarm = MidiUtilAlarm_new();

	for (i = 0; i < 128; i++)
	{
		note_commands[i] = NULL;
		controller_commands[i] = NULL;
		controller_hold_commands[i] = NULL;
		controller_state[i] = 0;
		controller_hold_ids[i] = NULL;
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("onmidi", argv[i], "onmidi", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("onmidi", argv[i], "onmidi")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--hold-length") == 0)
		{
			if (++i == argc) usage(argv[0]);
			hold_length_msecs = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--note-command") == 0)
		{
			int note;
			if (++i == argc) usage(argv[0]);
			note = MidiUtil_getNoteNumberFromName(argv[i]);
			if (++i == argc) usage(argv[0]);
			note_commands[note] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-hold-command") == 0)
		{
			int number;
			if (++i == argc) usage(argv[0]);
			number = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_hold_commands[number] = argv[i];
		}
		else if (strcmp(argv[i], "--pitch-wheel-up-command") == 0)
		{
			if (++i == argc) usage(argv[0]);
			pitch_wheel_up_command = argv[i];
		}
		else if (strcmp(argv[i], "--pitch-wheel-down-command") == 0)
		{
			if (++i == argc) usage(argv[0]);
			pitch_wheel_down_command = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_in == NULL) usage(argv[0]);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


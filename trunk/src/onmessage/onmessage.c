
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>

int midi_in_number = -1;
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
HMIDIIN midi_in = 0;
int shutting_down = 0;

void usage(char *app_name)
{
	fprintf(stderr, "Usage: %s --in <n> ( --note-command <number> <command> | --note-down-command <number> <command> | --note-up-command <number> <command> | --controller-command <number> <command> | --controller-down-command <number> <command> | --controller-up-command <number> <command> | --controller-increase-command <number> <command> | --controller-decrease-command <number> <command> | --program-command <number> <command> ) ...\n", app_name);
	exit(1);
}

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD timestamp)
{
	if (shutting_down) return;

	if (msg_type == MIM_DATA)
	{
		union
		{
			DWORD dwData;
			BYTE bData[4];
		}
		u;

		u.dwData = midi_msg;

		switch (u.bData[0] & 0xF0)
		{
			case 0x80:
			{
				int note = u.bData[1];
				if (note_commands[note] != NULL) system(note_commands[note]);
				if (note_up_commands[note] != NULL) system(note_up_commands[note]);
				break;
			}
			case 0x90:
			{
				int note = u.bData[1];
				int velocity = u.bData[2];
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
			case 0xB0:
			{
				int controller = u.bData[1];
				int value = u.bData[2];
				int previous_value = previous_controller_value[controller];
				if (controller_commands[controller] != NULL) system(controller_commands[controller]);
				if ((value >= 64) && (previous_value < 64) && (controller_down_commands[controller] != NULL)) system(controller_down_commands[controller]);
				if ((value < 64) && (previous_value >= 64) && (controller_up_commands[controller] != NULL)) system(controller_up_commands[controller]);

				if (previous_value >= 0)
				{
					if ((value > previous_value) && (controller_increase_commands[controller] != NULL)) system(controller_increase_commands[controller]);
					if ((value < previous_value) && (controller_decrease_commands[controller] != NULL)) system(controller_decrease_commands[controller]);
				}

				previous_controller_value[controller] = value;
				break;
			}
			case 0xC0:
			{
				int program = u.bData[1];
				if (program_commands[program] != NULL) system(program_commands[program]);
				break;
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	shutting_down = 1;
	midiInStop(midi_in);
	midiInClose(midi_in);
	return FALSE;
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
			midi_in_number = atoi(argv[i]);
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
			int controller;
			if (++i == argc) usage(argv[0]);
			controller = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_commands[controller] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-down-command") == 0)
		{
			int controller;
			if (++i == argc) usage(argv[0]);
			controller = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_down_commands[controller] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-up-command") == 0)
		{
			int controller;
			if (++i == argc) usage(argv[0]);
			controller = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_up_commands[controller] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-increase-command") == 0)
		{
			int controller;
			if (++i == argc) usage(argv[0]);
			controller = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_increase_commands[controller] = argv[i];
		}
		else if (strcmp(argv[i], "--controller-decrease-command") == 0)
		{
			int controller;
			if (++i == argc) usage(argv[0]);
			controller = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			controller_decrease_commands[controller] = argv[i];
		}
		else if (strcmp(argv[i], "--program-command") == 0)
		{
			int program;
			if (++i == argc) usage(argv[0]);
			program = atoi(argv[i]);
			if (++i == argc) usage(argv[0]);
			program_commands[program] = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_in_number < 0) usage(argv[0]);

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(0), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Error: Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	midiInStart(midi_in);

	SetConsoleCtrlHandler(control_handler, TRUE);

	Sleep(INFINITE);
	return 0;
}


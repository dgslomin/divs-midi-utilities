
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include <simplealarm.h>

int midi_in_number = -1;
char *command = NULL;
int hold_length = 500;
char *hold_command = NULL;
SimpleAlarm_t alarm = NULL;
HMIDIIN midi_in = 0;
int is_pressed = 0;
int alarm_rang = 0;

void usage(char *app_name)
{
	printf("Usage: %s --in <n> --command <str> [--hold-length <n>] [--hold-command <str>]\n", app_name);
	exit(1);
}

void alarm_handler(void *user_data)
{
	alarm_rang = 1;
	system(hold_command);
}

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD timestamp)
{
	if (msg_type == MIM_DATA)
	{
		union
		{
			DWORD dwData;
			BYTE bData[4];
		}
		u;

		u.dwData = midi_msg;

		if (((u.bData[0] & 0xF0) == 0xB0) && (u.bData[1] == 64))
		{
			if (u.bData[2] >= 64)
			{
				if (!is_pressed)
				{
					is_pressed = 1;

					if (hold_command != NULL)
					{
						alarm_rang = 0;
						SimpleAlarm_set(alarm, hold_length, alarm_handler, NULL);
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
						SimpleAlarm_cancel(alarm);
						system(command);
					}
				}
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	midiInStop(midi_in);
	midiInClose(midi_in);
	SimpleAlarm_free(alarm);
	return FALSE;
}

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &midi_in_number);
		}
		else if (strcmp(argv[i], "--command") == 0)
		{
			i++;
			if (i >= argc) break;
			command = argv[i];
		}
		else if (strcmp(argv[i], "--hold-length") == 0)
		{
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &hold_length);
		}
		else if (strcmp(argv[i], "--hold-command") == 0)
		{
			i++;
			if (i >= argc) break;
			hold_command = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in_number < 0) || (command == NULL)) usage(argv[0]);

	alarm = SimpleAlarm_new();

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(0), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	midiInStart(midi_in);

	SetConsoleCtrlHandler(control_handler, TRUE);

	Sleep(INFINITE);
	return 0;
}


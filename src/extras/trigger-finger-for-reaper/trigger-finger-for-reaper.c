
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>

int midi_in_number = -1;
int midi_out_number = -1;
HMIDIIN midi_in = 0;
HMIDIOUT midi_out = 0;
int shutting_down = 0;
int key_down = 0;
int last_controller_value[128];

void usage(char *app_name)
{
	fprintf(stderr, "Usage: %s --in <n> --out <n>\n", app_name);
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

		if ((u.bData[0] & 0xF0) == 0x90)
		{
			key_down = (u.bData[2] != 0x00);
		}
		else if ((u.bData[0] & 0xF0) == 0x80)
		{
			key_down = 0;
		}
		else if ((u.bData[0] & 0xF0) == 0xB0)
		{
			int controller = u.bData[1];
			int value = u.bData[2];

			if (!key_down)
			{
				if (value < last_controller_value[controller])
				{
					u.bData[2] = 0x00;
					midiOutShortMsg(midi_out, u.dwData);
				}
				else if (value > last_controller_value[controller])
				{
					u.bData[2] = 0x7F;
					midiOutShortMsg(midi_out, u.dwData);
				}

				last_controller_value[controller] = value;
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	shutting_down = 1;
	midiInStop(midi_in);
	midiInClose(midi_in);
	midiOutClose(midi_out);
	return FALSE;
}

int main(int argc, char **argv)
{
	int i;

	for (i = 0; i < 128; i++) last_controller_value[i] = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_in_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_out_number = atoi(argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in_number < 0) || (midi_out_number < 0)) usage(argv[0]);

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(0), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Error: Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Error: Cannot open MIDI output port #%d.\n", midi_out_number);
		exit(1);
	}

	midiInStart(midi_in);

	SetConsoleCtrlHandler(control_handler, TRUE);

	Sleep(INFINITE);
	return 0;
}


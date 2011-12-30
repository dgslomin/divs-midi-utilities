
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int controller_number = 7;
int controller_value = 0;
int inverted = 0;
HMIDIIN midi_in;
HMIDIOUT midi_out;

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
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

		switch (u.bData[0] & 0xF0)
		{
			case 0x80:
			case 0x90:
			{
				u.bData[2] = (BYTE)(((int)(u.bData[2]) * (inverted ? (0x7F - controller_value) : controller_value)) / 0x7F);
				midiOutShortMsg(midi_out, u.dwData);
				break;
			}
			case 0xB0:
			{
				if (u.bData[1] == controller_number)
				{
					controller_value = u.bData[2];
				}
				else
				{
					midiOutShortMsg(midi_out, u.dwData);
				}

				break;
			}
			case 0xA0:
			case 0xC0:
			case 0xD0:
			case 0xE0:
			{
				midiOutShortMsg(midi_out, u.dwData);
				break;
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	midiInStop(midi_in);
	midiInClose(midi_in);
	midiOutClose(midi_out);
	return FALSE;
}

int main(int argc, char **argv)
{
	int i;
	int midi_in_number = 0;
	int midi_out_number = MIDI_MAPPER;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &midi_in_number);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &midi_out_number);
		}
		else if (strcmp(argv[i], "--controller") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &controller_number);
		}
		else if (strcmp(argv[i], "--inverted") == 0)
		{
			inverted = 1;
		}
		else
		{
			printf("Usage: %s [--in <n>] [--out <n>] [--controller <n>] [--inverted]\n", argv[0]);
			return 1;
		}
	}

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	midiInStart(midi_in);

	Sleep(INFINITE);
	return 0;
}


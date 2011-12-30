
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DWORD note_on_msg, note_off_msg;
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

		if (((u.bData[0] & 0xF0) == 0xB0) && (u.bData[1] == 64))
		{
			if (u.bData[2] >= 64)
			{
				midiOutShortMsg(midi_out, note_on_msg);
			}
			else
			{
				midiOutShortMsg(midi_out, note_off_msg);
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
	int i, midi_in_number = 0, midi_out_number = MIDI_MAPPER, channel = 0, note = 64, velocity = 64;

	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

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
		else if (strcmp(argv[i], "--channel") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &channel);
		}
		else if (strcmp(argv[i], "--note") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &note);
		}
		else if (strcmp(argv[i], "--velocity") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &velocity);
		}
		else
		{
			printf("Usage: %s [--in <n>] [--out <n>] [--channel <n>] [--note <n>] [--velocity <n>]\n", argv[0]);
			return 1;
		}
	}

	u.bData[0] = 0x90 | channel;
	u.bData[1] = (BYTE)(note);
	u.bData[2] = (BYTE)(velocity);
	u.bData[3] = 0;
	note_on_msg = u.dwData;

	u.bData[0] = 0x80 | channel;
	u.bData[1] = (BYTE)(note);
	u.bData[2] = 0;
	u.bData[3] = 0;
	note_off_msg = u.dwData;

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.", midi_out_number);
		return 1;
	}

	SetConsoleCtrlHandler(control_handler, TRUE);
	midiInStart(midi_in);
	Sleep(INFINITE);
	return 0;
}


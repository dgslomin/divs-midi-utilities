
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

struct Echo
{
	int delay;
	int note_interval;
	int velocity_percent;
	struct Echo *next_echo;
};

typedef struct Echo *Echo_t;

HMIDIIN midi_in;
HMIDIOUT midi_out;
HANDLE timer_queue;
Echo_t first_echo;

int clamp(int n, int low, int high)
{
	if (n < low) return low;
	if (n > high) return high;
	return n;
}

VOID CALLBACK timer_handler(PVOID *user_data, BOOLEAN param2)
{
	DWORD *midi_msg_p = (DWORD *)(user_data);
	midiOutShortMsg(midi_out, *midi_msg_p);
	free(midi_msg_p);
}

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
				Echo_t echo;

				for (echo = first_echo; echo != NULL; echo = echo->next_echo)
				{
					u.dwData = midi_msg;
					u.bData[1] = clamp((int)(u.bData[1]) + (echo->note_interval), 0, 127);
					u.bData[2] = clamp((int)(u.bData[2]) * (echo->velocity_percent) / 100, 0, 127);

					if (echo->delay == 0)
					{
						midiOutShortMsg(midi_out, u.dwData);
					}
					else
					{
						DWORD *midi_msg_copy_p;
						HANDLE timer;

						midi_msg_copy_p = malloc(sizeof(DWORD));
						*midi_msg_copy_p = u.dwData;
						CreateTimerQueueTimer(&timer, timer_queue, timer_handler, midi_msg_copy_p, echo->delay, 0, 0);
					}
				}

				break;
			}
			case 0xA0:
			case 0xB0:
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

	first_echo = NULL;

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
		else if (strcmp(argv[i], "--echo") == 0)
		{
			int delay = 0;
			int note_interval = 0;
			int velocity_percent = 100;
			Echo_t echo;

			if (++i >= argc) break;
			sscanf(argv[i], "%d", &delay);
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &note_interval);
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &velocity_percent);

			echo = malloc(sizeof(struct Echo));
			echo->delay = delay;
			echo->note_interval = note_interval;
			echo->velocity_percent = velocity_percent;
			echo->next_echo = first_echo;
			first_echo = echo;
		}
		else
		{
			printf("Usage: %s [--in <n>] [--out <n>] [ --echo <delay in ms> <note interval> <velocity percent> ... ]\n", argv[0]);
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

	timer_queue = CreateTimerQueue();

	midiInStart(midi_in);

	Sleep(INFINITE);
	return 0;
}


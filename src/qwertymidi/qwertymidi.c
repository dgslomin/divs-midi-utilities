
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define ACTION_NOOP 10000
#define ACTION_EXIT 10001
#define ACTION_PANIC 10002
#define ACTION_SHIFT_UP_OCTAVE 10003
#define ACTION_SHIFT_DOWN_OCTAVE 10004
#define ACTION_STAY_UP_OCTAVE 10005
#define ACTION_STAY_DOWN_OCTAVE 10006
#define ACTION_SHIFT_UP_NOTE 10007
#define ACTION_SHIFT_DOWN_NOTE 10008
#define ACTION_STAY_UP_NOTE 10009
#define ACTION_STAY_DOWN_NOTE 10010

static int clamp(int i, int low, int high)
{
	return i < low ? low : i > high ? high : i;
}

static void send_midi_note_on(HMIDIOUT midi_out, int channel, int pitch, int velocity)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x90 | (BYTE)(channel);
	u.bData[1] = (BYTE)(pitch);
	u.bData[2] = (BYTE)(velocity);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static void send_midi_note_off(HMIDIOUT midi_out, int channel, int pitch, int velocity)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x80 | (BYTE)(channel);
	u.bData[1] = (BYTE)(pitch);
	u.bData[2] = (BYTE)(velocity);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static void send_midi_program_change(HMIDIOUT midi_out, int channel, int program_number)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xC0 | (BYTE)(channel);
	u.bData[1] = (BYTE)(program_number);
	u.bData[2] = 0;
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

int main(int argc, char **argv)
{
	int i;
	int map[128];
	int down[128];
	int midi_out_number = -1;
	int channel_number = 0;
	int program_number = -1;
	int velocity = 64;
	char *map_filename = NULL;
	HMIDIOUT midi_out;
	HANDLE console_input;
	DWORD console_mode;
	INPUT_RECORD input_record;
	DWORD records_read;
	int done = 0;
	int transposition = 0;

	for (i = 0; i < 128; i++)
	{
		map[i] = ACTION_NOOP;
		down[i] = 0;
	}

	map[1] = ACTION_EXIT;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &midi_out_number);
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &channel_number);
		}
		else if (strcmp(argv[i], "--program") == 0)
		{
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &program_number);
		}
		else if (strcmp(argv[i], "--velocity") == 0)
		{
			i++;
			if (i >= argc) break;
			sscanf(argv[i], "%d", &velocity);
		}
		else if (strcmp(argv[i], "--map") == 0)
		{
			i++;
			if (i >= argc) break;
			map_filename = argv[i];
		}
		else
		{
			printf("Usage: %s [--out <n>] [--channel <n>] [--program <n>] [--velocity <n>] [--map <str>]\n", argv[0]);
			return 1;
		}
	}

	if (map_filename != NULL)
	{
		FILE *map_file = fopen(map_filename, "r");
		char line[1024];
		int scan_code = 0;
		char arg[1024];
		int note = 0;

		if (map_file == NULL)
		{
			printf("Cannot open map file \"%s\"\n", map_filename);
			return 1;
		}

		while (fgets(line, 1024, map_file) != NULL)
		{
			if (sscanf(line, " %d %s", &scan_code, arg) != 2) continue;

			scan_code = clamp(scan_code, 0, 127);

			if (strcmp(arg, "noop") == 0)
			{
				map[scan_code] = ACTION_NOOP;
			}
			else if (strcmp(arg, "exit") == 0)
			{
				map[scan_code] = ACTION_EXIT;
			}
			else if (strcmp(arg, "panic") == 0)
			{
				map[scan_code] = ACTION_PANIC;
			}
			else if ((strcmp(arg, "shift-up-octave") == 0) || (strcmp(arg, "shift-up") == 0))
			{
				map[scan_code] = ACTION_SHIFT_UP_OCTAVE;
			}
			else if ((strcmp(arg, "shift-down-octave") == 0) || (strcmp(arg, "shift-down") == 0))
			{
				map[scan_code] = ACTION_SHIFT_DOWN_OCTAVE;
			}
			else if ((strcmp(arg, "stay-up-octave") == 0) || (strcmp(arg, "stay-up") == 0))
			{
				map[scan_code] = ACTION_STAY_UP_OCTAVE;
			}
			else if ((strcmp(arg, "stay-down-octave") == 0) || (strcmp(arg, "stay-down") == 0))
			{
				map[scan_code] = ACTION_STAY_DOWN_OCTAVE;
			}
			else if (strcmp(arg, "shift-up-note") == 0)
			{
				map[scan_code] = ACTION_SHIFT_UP_NOTE;
			}
			else if (strcmp(arg, "shift-down-note") == 0)
			{
				map[scan_code] = ACTION_SHIFT_DOWN_NOTE;
			}
			else if (strcmp(arg, "stay-up-note") == 0)
			{
				map[scan_code] = ACTION_STAY_UP_NOTE;
			}
			else if (strcmp(arg, "stay-down-note") == 0)
			{
				map[scan_code] = ACTION_STAY_DOWN_NOTE;
			}
			else
			{
				if (sscanf(arg, "%d", &note) != 1) continue;
				map[scan_code] = note;
			}
		}
	}

	if (midi_out_number < 0) midi_out_number = MIDI_MAPPER;
	channel_number = clamp(channel_number, 0, 15);
	program_number = clamp(program_number, -1, 127);
	velocity = clamp(velocity, 0, 127);

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	if (program_number >= 0)
	{
		send_midi_program_change(midi_out, channel_number, program_number);
	}

	console_input = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(console_input, &console_mode);
	SetConsoleMode(console_input, console_mode & ~ENABLE_PROCESSED_INPUT);

	while (!done)
	{
		ReadConsoleInput(console_input, &input_record, 1, &records_read);

		if (input_record.EventType == KEY_EVENT)
		{
			int action = map[input_record.Event.KeyEvent.wVirtualScanCode];

			if (input_record.Event.KeyEvent.bKeyDown)
			{
				if (!down[input_record.Event.KeyEvent.wVirtualScanCode])
				{
					down[input_record.Event.KeyEvent.wVirtualScanCode] = 1;

					switch (action)
					{
						case ACTION_NOOP:
						{
							break;
						}
						case ACTION_EXIT:
						{
							done = 1;
							break;
						}
						case ACTION_PANIC:
						{
							for (i = 0; i < 127; i++) send_midi_note_off(midi_out, channel_number, i, 0);
							break;
						}
						case ACTION_SHIFT_UP_OCTAVE:
						case ACTION_STAY_UP_OCTAVE:
						{
							transposition += 12;
							break;
						}
						case ACTION_SHIFT_DOWN_OCTAVE:
						case ACTION_STAY_DOWN_OCTAVE:
						{
							transposition -= 12;
							break;
						}
						case ACTION_SHIFT_UP_NOTE:
						case ACTION_STAY_UP_NOTE:
						{
							transposition++;
							break;
						}
						case ACTION_SHIFT_DOWN_NOTE:
						case ACTION_STAY_DOWN_NOTE:
						{
							transposition--;
							break;
						}
						default:
						{
							send_midi_note_on(midi_out, channel_number, clamp(action + transposition, 0, 127), velocity);
							break;
						}
					}
				}

				/* Allow caps, number, and scroll lock to be used as a trigger keys without affecting the lock state. */

				if (input_record.Event.KeyEvent.wVirtualScanCode == 58)
				{
					keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_CAPITAL, 0, 0, 0);
					keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (input_record.Event.KeyEvent.wVirtualScanCode == 69)
				{
					keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_NUMLOCK, 0, 0, 0);
					keybd_event(VK_NUMLOCK, 0, KEYEVENTF_KEYUP, 0);
				}
				else if (input_record.Event.KeyEvent.wVirtualScanCode == 70)
				{
					keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
					keybd_event(VK_SCROLL, 0, 0, 0);
					keybd_event(VK_SCROLL, 0, KEYEVENTF_KEYUP, 0);
				}
			}
			else
			{
				down[input_record.Event.KeyEvent.wVirtualScanCode] = 0;

				switch (action)
				{
					case ACTION_NOOP:
					case ACTION_EXIT:
					case ACTION_PANIC:
					case ACTION_STAY_UP_OCTAVE:
					case ACTION_STAY_DOWN_OCTAVE:
					case ACTION_STAY_UP_NOTE:
					case ACTION_STAY_DOWN_NOTE:
					{
						break;
					}
					case ACTION_SHIFT_UP_OCTAVE:
					{
						transposition -= 12;
						break;
					}
					case ACTION_SHIFT_DOWN_OCTAVE:
					{
						transposition += 12;
						break;
					}
					case ACTION_SHIFT_UP_NOTE:
					{
						transposition--;
						break;
					}
					case ACTION_SHIFT_DOWN_NOTE:
					{
						transposition++;
						break;
					}
					default:
					{
						send_midi_note_off(midi_out, channel_number, clamp(action + transposition, 0, 127), 0);
						break;
					}
				}
			}
		}
	}

	midiOutClose(midi_out);
	return 0;
}


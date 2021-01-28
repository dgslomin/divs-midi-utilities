
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define ACTION_EXIT 10000
#define ACTION_REST 10001

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
	int midi_out_number = -1;
	int channel_number = 0;
	int program_number = -1;
	int velocity = 64;
	char *map_filename = NULL;
	HMIDIOUT midi_out;
	HANDLE console_input;
	INPUT_RECORD input_record;
	DWORD records_read;
	int done = 0;
	int current_note = 63;

	for (i = 0; i < 128; i++) map[i] = ACTION_REST;
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
		int interval = 0;

		if (map_file == NULL)
		{
			printf("Cannot open map file \"%s\"\n", map_filename);
			return 1;
		}

		while (fgets(line, 1024, map_file) != NULL)
		{
			if (sscanf(line, " %d %s", &scan_code, arg) != 2) continue;

			scan_code = clamp(scan_code, 0, 127);

			if (strcmp(arg, "exit") == 0)
			{
				map[scan_code] = ACTION_EXIT;
			}
			else if (strcmp(arg, "rest") == 0)
			{
				map[scan_code] = ACTION_REST;
			}
			else
			{
				if (sscanf(arg, "%d", &interval) != 1) continue;
				map[scan_code] = interval;
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

	while (!done)
	{
		ReadConsoleInput(console_input, &input_record, 1, &records_read);

		if ((input_record.EventType == KEY_EVENT) && input_record.Event.KeyEvent.bKeyDown)
		{
			int action = map[input_record.Event.KeyEvent.wVirtualScanCode];

			switch (action)
			{
				case ACTION_EXIT:
				{
					send_midi_note_off(midi_out, channel_number, current_note, 0);
					done = 1;
					break;
				}
				case ACTION_REST:
				{
					send_midi_note_off(midi_out, channel_number, current_note, 0);
					break;
				}
				default:
				{
					send_midi_note_off(midi_out, channel_number, current_note, 0);
					current_note = clamp(current_note + action, 0, 127);
					send_midi_note_on(midi_out, channel_number, current_note, velocity);
					break;
				}
			}
		}
	}

	midiOutClose(midi_out);
	return 0;
}


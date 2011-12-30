
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef enum
{
	KNOB_NONE = -1
};

typedef enum
{
	CONTROLLER_TYPE_NONE = -1,
	CONTROLLER_TYPE_CC,
	CONTROLLER_TYPE_RPN,
	CONTROLLER_TYPE_NRPN
}
CONTROLLER_TYPE;

typedef enum
{
	CONTROLLER_NUMBER_NONE = -1
};

struct knob
{
	CONTROLLER_TYPE controller_type;
	int controller_number;
	int controller_value;
	int has_positive_thrust;
	int has_negative_thrust;
	float velocity;
	float position;
};

static int clamp(int i, int low, int high)
{
	return i < low ? low : i > high ? high : i;
}

static float clampf(float i, float low, float high)
{
	return i < low ? low : i > high ? high : i;
}

static void send_cc(HMIDIOUT midi_out, int channel, int number, int value)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xB0 | (BYTE)(channel);
	u.bData[1] = (BYTE)(number);
	u.bData[2] = (BYTE)(value);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

static void send_rpn(HMIDIOUT midi_out, int channel, int number, int value)
{
	send_cc(midi_out, channel, 101, number >> 7);
	send_cc(midi_out, channel, 6, number >> 7);
	send_cc(midi_out, channel, 100, number & 0x7F);
	send_cc(midi_out, channel, 38, number & 0x7F);
}

static void send_nrpn(HMIDIOUT midi_out, int channel, int number, int value)
{
	send_cc(midi_out, channel, 99, number >> 7);
	send_cc(midi_out, channel, 6, number >> 7);
	send_cc(midi_out, channel, 98, number & 0x7F);
	send_cc(midi_out, channel, 38, number & 0x7F);
}

int channel_number = 0;
HMIDIOUT midi_out;
int done = 0;
struct knob knobs[22];
int fast = 0;
int slow = 0;

int thread_main(void *user_data)
{
	while (!done)
	{
		int i;

		for (i = 0; i < 22; i++)
		{
			int previous_value = 0;
			int new_value = 0;

			switch (knobs[i].controller_type)
			{
				case CONTROLLER_TYPE_CC:
				{
					previous_value = knobs[i].position * 128;
					break;
				}
				case CONTROLLER_TYPE_RPN:
				case CONTROLLER_TYPE_NRPN:
				{
					previous_value = knobs[i].position * (2 << 14);
					break;
				}
				default:
				{
					break;
				}
			}

			if (knobs[i].has_positive_thrust)
			{
				if (!knobs[i].has_negative_thrust)
				{
					if (fast)
					{
						if (slow)
						{
							knobs[i].velocity = clampf(knobs[i].velocity + 0.1, -1.0, 1.0);
						}
						else
						{
							knobs[i].velocity = clampf(knobs[i].velocity + 0.2, -1.0, 1.0);
						}
					}
					else
					{
						if (slow)
						{
							knobs[i].velocity = clampf(knobs[i].velocity + 0.05, -1.0, 1.0);
						}
						else
						{
							knobs[i].velocity = clampf(knobs[i].velocity + 0.1, -1.0, 1.0);
						}
					}
				}
			}
			else
			{
				if (knobs[i].has_negative_thrust)
				{
					if (fast)
					{
						if (slow)
						{
							knobs[i].velocity = clampf(knobs[i].velocity - 0.1, -1.0, 1.0);
						}
						else
						{
							knobs[i].velocity = clampf(knobs[i].velocity - 0.2, -1.0, 1.0);
						}
					}
					else
					{
						if (slow)
						{
							knobs[i].velocity = clampf(knobs[i].velocity - 0.05, -1.0, 1.0);
						}
						else
						{
							knobs[i].velocity = clampf(knobs[i].velocity - 0.1, -1.0, 1.0);
						}
					}
				}
			}

			if (knobs[i].velocity > 0)
			{
				knobs[i].velocity = clampf(knobs[i].velocity - 0.01, -1.0, 1.0);
			}
			else if (knobs[i].velocity < 0)
			{
				knobs[i].velocity = clampf(knobs[i].velocity + 0.01, -1.0, 1.0);
			}

			knobs[i].position = clampf(knobs[i].position + knobs[i].velocity, 0.0, 1.0);

			switch (knobs[i].controller_type)
			{
				case CONTROLLER_TYPE_CC:
				{
					new_value = knobs[i].position * 128;
					break;
				}
				case CONTROLLER_TYPE_RPN:
				case CONTROLLER_TYPE_NRPN:
				{
					new_value = knobs[i].position * (2 << 14);
					break;
				}
				default:
				{
					break;
				}
			}

			if (new_value != previous_value)
			{
				switch (knobs[i].controller_type)
				{
					case CONTROLLER_TYPE_CC:
					{
						send_cc(midi_out, channel_number, knobs[i].controller_number, new_value);
						break;
					}
					case CONTROLLER_TYPE_RPN:
					{
						send_rpn(midi_out, channel_number, knobs[i].controller_number, new_value);
						break;
					}
					case CONTROLLER_TYPE_NRPN:
					{
						send_nrpn(midi_out, channel_number, knobs[i].controller_number, new_value);
						break;
					}
					default:
					{
						break;
					}
				}
			}
		}

		Sleep(10);
	}

	return 0;
}

void usage(char *program_name)
{
	printf("Usage: %s [ --out <n> ] [ --channel <n> ] [ --knob <n> ( --cc <number> | --rpn <number> | --nrpn <number> ) ] ...\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int scan_code_to_knob_map[128];
	int scan_code_corresponds_to_positive_thrust[128];
	int midi_out_number = -1;
	int knob_number = -1;
	HANDLE console_input;
	HANDLE thread;

	for (i = 0; i < 128; i++)
	{
		scan_code_to_knob_map[i] = KNOB_NONE;
		scan_code_corresponds_to_positive_thrust[i] = 0;
	}

	for (i = 0; i < 10; i++)
	{
		scan_code_to_knob_map[i + 30] = i;
		scan_code_corresponds_to_positive_thrust[i + 30] = 1;

		scan_code_to_knob_map[i + 44] = i;
		scan_code_corresponds_to_positive_thrust[i + 44] = 0;
	}

	for (i = 0; i < 12; i++)
	{
		scan_code_to_knob_map[i + 2] = i + 10;
		scan_code_corresponds_to_positive_thrust[i + 2] = 1;

		scan_code_to_knob_map[i + 16] = i + 10;
		scan_code_corresponds_to_positive_thrust[i + 16] = 0;
	}

	for (i = 0; i < 22; i++)
	{
		knobs[i].controller_type = CONTROLLER_TYPE_NONE;
		knobs[i].controller_number = CONTROLLER_NUMBER_NONE;
		knobs[i].has_positive_thrust = 0;
		knobs[i].has_negative_thrust = 0;
		knobs[i].velocity = 0.0;
		knobs[i].position = 0.0;
	}

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			i++;
			if (i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &midi_out_number);
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			i++;
			if (i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &channel_number);
		}
		else if (strcmp(argv[i], "--knob") == 0)
		{
			i++;
			if (i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &knob_number);
		}
		else if (strcmp(argv[i], "--cc") == 0)
		{
			int controller_number;
			i++;
			if (i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &controller_number);
			if ((knob_number < 0) || (knob_number >= 22)) usage(argv[0]);
			knobs[knob_number].controller_type = CONTROLLER_TYPE_CC;
			knobs[knob_number].controller_number = controller_number;
		}
		else if (strcmp(argv[i], "--rpn") == 0)
		{
			int controller_number;
			i++;
			if (i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &controller_number);
			if (knob_number < 0) usage(argv[0]);
			knobs[knob_number].controller_type = CONTROLLER_TYPE_RPN;
			knobs[knob_number].controller_number = controller_number;
		}
		else if (strcmp(argv[i], "--nrpn") == 0)
		{
			int controller_number;
			i++;
			if (i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &controller_number);
			if (knob_number < 0) usage(argv[0]);
			knobs[knob_number].controller_type = CONTROLLER_TYPE_NRPN;
			knobs[knob_number].controller_number = controller_number;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_out_number < 0) midi_out_number = MIDI_MAPPER;
	channel_number = clamp(channel_number, 0, 15);

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	console_input = GetStdHandle(STD_INPUT_HANDLE);

	_beginthreadex(NULL, 0, &thread_main, NULL, 0, &thread);

	while (!done)
	{
		INPUT_RECORD input_record;
		DWORD records_read;

		ReadConsoleInput(console_input, &input_record, 1, &records_read);

		if (input_record.EventType == KEY_EVENT)
		{
			int knob = scan_code_to_knob_map[input_record.Event.KeyEvent.wVirtualScanCode];

			if (knob >= 0)
			{
				if (input_record.Event.KeyEvent.bKeyDown)
				{
					if (scan_code_corresponds_to_positive_thrust[input_record.Event.KeyEvent.wVirtualScanCode])
					{
						knobs[knob].has_positive_thrust = 1;
					}
					else
					{
						knobs[knob].has_negative_thrust = 1;
					}
				}
				else
				{
					if (scan_code_corresponds_to_positive_thrust[input_record.Event.KeyEvent.wVirtualScanCode])
					{
						knobs[knob].has_positive_thrust = 0;
					}
					else
					{
						knobs[knob].has_negative_thrust = 0;
					}
				}
			}
			else if (input_record.Event.KeyEvent.wVirtualScanCode == 42)
			{
				fast = input_record.Event.KeyEvent.bKeyDown;
			}
			else if (input_record.Event.KeyEvent.wVirtualScanCode == 29)
			{
				slow = input_record.Event.KeyEvent.bKeyDown;
			}
			else if (input_record.Event.KeyEvent.wVirtualScanCode == 1)
			{
				done = 1;
			}
		}
	}

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);

	midiOutClose(midi_out);
	return 0;
}


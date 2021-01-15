
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

static void send_midi_control_change(HMIDIOUT midi_out, int channel_number, int controller_number, int value)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xB0 | (BYTE)(channel_number);
	u.bData[1] = (BYTE)(controller_number);
	u.bData[2] = (BYTE)(value);
	u.bData[3] = 0;

	midiOutShortMsg(midi_out, u.dwData);
}

int main(int argc, char **argv)
{
	int i;
	int midi_out_number = MIDI_MAPPER;
	int channel_number = 0;
	int controller_number = 7;
	int enable_graph = 0;

	JOYINFO joy_info;
	HMIDIOUT midi_out;
	HANDLE console_input = GetStdHandle(STD_INPUT_HANDLE);
	int number_of_input_events;
	INPUT_RECORD input_record;
	int zero_position = -1;
	int max_position = -1;
	int last_value = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &midi_out_number);
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &channel_number);
		}
		else if (strcmp(argv[i], "--controller") == 0)
		{
			if (++i >= argc) break;
			sscanf(argv[i], "%d", &controller_number);
		}
		else if (strcmp(argv[i], "--graph") == 0)
		{
			enable_graph = 1;
		}
		else
		{
			printf("Usage: %s [--out <n>] [--channel <n>] [--controller <n>] [--graph]\n", argv[0]);
			return 1;
		}
	}

	switch (joyGetPos(JOYSTICKID1, &joy_info))
	{
		case JOYERR_NOERROR:
		{
			/* good */
			break;
		}
		case MMSYSERR_NODRIVER:
		{
			printf("No joystick/pedal is installed.\n");
			return 1;
		}
		case MMSYSERR_INVALPARAM:
		{
			printf("Internal error.\n");
			return 1;
		}
		case JOYERR_UNPLUGGED:
		{
			printf("The joystick/pedal is unplugged.\n");
			return 1;
		}
		default:
		{
			printf("Internal error.\n");
			return 1;
		}
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	printf("Set joystick/pedal to max and press space.\n");

	while (1)
	{
		if (joyGetPos(JOYSTICKID1, &joy_info) == JOYERR_NOERROR) max_position = joy_info.wXpos;
		if (GetNumberOfConsoleInputEvents(console_input, &number_of_input_events) && (number_of_input_events > 0) && ReadConsoleInput(console_input, &input_record, 1, &number_of_input_events) && (input_record.EventType == KEY_EVENT) && input_record.Event.KeyEvent.bKeyDown && (input_record.Event.KeyEvent.wVirtualScanCode == 0x39)) break;
		Sleep(5);
	}

	if (max_position < 0)
	{
		printf("Cannot read from joystick/pedal.\n");
		return 1;
	}

	printf("Set joystick/pedal to zero and press space.\n");

	while (1)
	{
		if (joyGetPos(JOYSTICKID1, &joy_info) == JOYERR_NOERROR) zero_position = joy_info.wXpos;
		if (GetNumberOfConsoleInputEvents(console_input, &number_of_input_events) && (number_of_input_events > 0) && ReadConsoleInput(console_input, &input_record, 1, &number_of_input_events) && (input_record.EventType == KEY_EVENT) && input_record.Event.KeyEvent.bKeyDown && (input_record.Event.KeyEvent.wVirtualScanCode == 0x39)) break;
		Sleep(5);
	}

	if (zero_position < 0)
	{
		printf("Cannot read from joystick/pedal.\n");
		return 1;
	}

	if (zero_position == max_position)
	{
		printf("Could not detect any difference between zero and max.\n");
		return 1;
	}

	printf("Running.\n");

	while (1)
	{
		if (joyGetPos(JOYSTICKID1, &joy_info) == JOYERR_NOERROR)
		{
			int value;

			if (zero_position < max_position)
			{
				value = ((joy_info.wXpos - zero_position) * 128) / (max_position - zero_position);
			}
			else
			{
				value = ((zero_position - joy_info.wXpos) * 128) / (zero_position - max_position);
			}

			if ((value >= 0) && (value <= 0x7F) && (value != last_value))
			{
				send_midi_control_change(midi_out, channel_number, controller_number, value);
				last_value = value;

				if (enable_graph) printf("-------------------------------------------------------------------------------\n" + (79 - ((79 * value) / 128)));
			}
		}

		Sleep(5);
	}

	midiOutClose(midi_out);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

HMIDIOUT midi_out;

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s [--out <n>] [--channel <n>] [--x1 <n>] [--y1 <n>] [--z1 <n>] [--a1 <n>] [--b1 <n>] [--c1 <n>] [--d1 <n>] [--x2 <n>] [--y2 <n>] [--z2 <n>] [--a2 <n>] [--b2 <n>] [--c2 <n>] [--d2 <n>]\n", program_name); 
	exit(1);
}

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

BOOL WINAPI control_handler(DWORD control_type)
{
	midiOutClose(midi_out);
	return FALSE;
}

int main(int argc, char **argv)
{
	int i;
	int midi_out_number = MIDI_MAPPER;
	int channel_number = 0;
	int use_joystick_1 = 0;
	int controller_number_x1 = -1;
	int controller_number_y1 = -1;
	int controller_number_z1 = -1;
	int controller_number_a1 = -1;
	int controller_number_b1 = -1;
	int controller_number_c1 = -1;
	int controller_number_d1 = -1;
	int last_value_x1 = -1;
	int last_value_y1 = -1;
	int last_value_z1 = -1;
	int last_value_a1 = -1;
	int last_value_b1 = -1;
	int last_value_c1 = -1;
	int last_value_d1 = -1;
	int use_joystick_2 = 0;
	int controller_number_x2 = -1;
	int controller_number_y2 = -1;
	int controller_number_z2 = -1;
	int controller_number_a2 = -1;
	int controller_number_b2 = -1;
	int controller_number_c2 = -1;
	int controller_number_d2 = -1;
	int last_value_x2 = -1;
	int last_value_y2 = -1;
	int last_value_z2 = -1;
	int last_value_a2 = -1;
	int last_value_b2 = -1;
	int last_value_c2 = -1;
	int last_value_d2 = -1;
	JOYINFO joy_info;
	JOYCAPS joy_caps_1;
	JOYCAPS joy_caps_2;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sscanf(argv[i], "%d", &midi_out_number);
		}
		else if (strcmp(argv[i], "--channel") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sscanf(argv[i], "%d", &channel_number);
		}
		else if (strcmp(argv[i], "--x1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_x1);
		}
		else if (strcmp(argv[i], "--y1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_y1);
		}
		else if (strcmp(argv[i], "--z1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_z1);
		}
		else if (strcmp(argv[i], "--a1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_a1);
		}
		else if (strcmp(argv[i], "--b1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_b1);
		}
		else if (strcmp(argv[i], "--c1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_c1);
		}
		else if (strcmp(argv[i], "--d1") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_1 = 1;
			sscanf(argv[i], "%d", &controller_number_d1);
		}
		else if (strcmp(argv[i], "--x2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_x2);
		}
		else if (strcmp(argv[i], "--y2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_y2);
		}
		else if (strcmp(argv[i], "--z2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_z2);
		}
		else if (strcmp(argv[i], "--a2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_a2);
		}
		else if (strcmp(argv[i], "--b2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_b2);
		}
		else if (strcmp(argv[i], "--c2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_c2);
		}
		else if (strcmp(argv[i], "--d2") == 0)
		{
			if (++i == argc) usage(argv[0]);
			use_joystick_2 = 1;
			sscanf(argv[i], "%d", &controller_number_d2);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (!use_joystick_1 && !use_joystick_2)
	{
		fprintf(stderr, "Error:  You must assign a controller to at least one joystick axis or button.\n");
		exit(1);
	}

	if (use_joystick_1)
	{
		if (joyGetPos(JOYSTICKID1, &joy_info) != JOYERR_NOERROR)
		{
			fprintf(stderr, "Error:  Cannot access joystick #1.\n");
			exit(1);
		}

		joyGetDevCaps(JOYSTICKID1, &joy_caps_1, sizeof(JOYCAPS));
	}

	if (use_joystick_2)
	{
		if (joyGetPos(JOYSTICKID2, &joy_info) != JOYERR_NOERROR)
		{
			fprintf(stderr, "Error:  Cannot access joystick #2.\n");
			exit(1);
		}

		joyGetDevCaps(JOYSTICKID2, &joy_caps_2, sizeof(JOYCAPS));
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Error:  Cannot open MIDI output port #%d.\n", midi_out_number);
		exit(1);
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	while (1)
	{
		if ((use_joystick_1) && (joyGetPos(JOYSTICKID1, &joy_info) == JOYERR_NOERROR))
		{
			if (controller_number_x1 >= 0)
			{
				int value = (joy_info.wXpos - joy_caps_1.wXmin) * 128 / (joy_caps_1.wXmax - joy_caps_1.wXmin);

				if (value != last_value_x1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_x1, value);
					last_value_x1 = value;
				}
			}

			if (controller_number_y1 >= 0)
			{
				int value = (joy_info.wYpos - joy_caps_1.wYmin) * 128 / (joy_caps_1.wYmax - joy_caps_1.wYmin);

				if (value != last_value_y1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_y1, value);
					last_value_y1 = value;
				}
			}

			if (controller_number_z1 >= 0)
			{
				int value = (joy_info.wZpos - joy_caps_1.wZmin) * 128 / (joy_caps_1.wZmax - joy_caps_1.wZmin);

				if (value != last_value_z1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_z1, value);
					last_value_z1 = value;
				}
			}

			if (controller_number_a1 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON1) == JOY_BUTTON1) ? 127 : 0;

				if (value != last_value_a1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_a1, value);
					last_value_a1 = value;
				}
			}

			if (controller_number_b1 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON2) == JOY_BUTTON2) ? 127 : 0;

				if (value != last_value_b1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_b1, value);
					last_value_b1 = value;
				}
			}

			if (controller_number_c1 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON3) == JOY_BUTTON3) ? 127 : 0;

				if (value != last_value_c1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_c1, value);
					last_value_c1 = value;
				}
			}

			if (controller_number_d1 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON4) == JOY_BUTTON4) ? 127 : 0;

				if (value != last_value_d1)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_d1, value);
					last_value_d1 = value;
				}
			}
		}

		if ((use_joystick_2) && (joyGetPos(JOYSTICKID2, &joy_info) == JOYERR_NOERROR))
		{
			if (controller_number_x2 >= 0)
			{
				int value = (joy_info.wXpos - joy_caps_2.wXmin) * 128 / (joy_caps_2.wXmax - joy_caps_2.wXmin);

				if (value != last_value_x2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_x2, value);
					last_value_x2 = value;
				}
			}

			if (controller_number_y2 >= 0)
			{
				int value = (joy_info.wYpos - joy_caps_2.wYmin) * 128 / (joy_caps_2.wYmax - joy_caps_2.wYmin);

				if (value != last_value_y2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_y2, value);
					last_value_y2 = value;
				}
			}

			if (controller_number_z2 >= 0)
			{
				int value = (joy_info.wZpos - joy_caps_2.wZmin) * 128 / (joy_caps_2.wZmax - joy_caps_2.wZmin);

				if (value != last_value_z2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_z2, value);
					last_value_z2 = value;
				}
			}

			if (controller_number_a2 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON1) == JOY_BUTTON1) ? 127 : 0;

				if (value != last_value_a2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_a2, value);
					last_value_a2 = value;
				}
			}

			if (controller_number_b2 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON2) == JOY_BUTTON2) ? 127 : 0;

				if (value != last_value_b2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_b2, value);
					last_value_b2 = value;
				}
			}

			if (controller_number_c2 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON3) == JOY_BUTTON3) ? 127 : 0;

				if (value != last_value_c2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_c2, value);
					last_value_c2 = value;
				}
			}

			if (controller_number_d2 >= 0)
			{
				int value = ((joy_info.wButtons & JOY_BUTTON4) == JOY_BUTTON4) ? 127 : 0;

				if (value != last_value_d2)
				{
					send_midi_control_change(midi_out, channel_number, controller_number_d2, value);
					last_value_d2 = value;
				}
			}
		}

		Sleep(5);
	}

	midiOutClose(midi_out);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define CLOCK_RATE 10

HMIDIIN midi_in;
HMIDIOUT midi_out;
int tempo = 120.0;
int last_controller_value = 0;
int key_to_edit = -1;

enum
{
	STATE_PLAY,
	STATE_CHOOSE_KEY_TO_EDIT,
	STATE_CHOOSE_OUTPUT_NOTE,
	STATE_CHOOSE_PATTERN
}
state = STATE_PLAY;

struct
{
	int output_channel;
	int output_note;
	int output_velocity;
	int pattern[12];
	int is_pressed;
}
key_data[128];

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
			{
				key[u.bData[1]].is_pressed = 0;
				break;
			}
			case 0x90:
			{
				switch (state)
				{
					case STATE_PLAY:
					{
						break;
					}
					case STATE_CHOOSE_KEY_TO_EDIT:
					{
						key_to_edit = u.bData[1];
						break;
					}
					case STATE_CHOOSE_OUTPUT_NOTE:
					{
						key_data[key_to_edit].output_channel = u.bData[0] & 0x0F;
						key_data[key_to_edit].output_note = u.bData[1];
						key_data[key_to_edit].output_velocity = u.bData[2];
						break;
					}
					case STATE_CHOOSE_PATTERN:
					{
						if ((u.bData[1] >= 60) && (u.bData[1] < 72))
						{
							key_data[key_to_edit].pattern[u.bData[1] - 60] = 1;
						}

						break;
					}
				}

				key[u.bData[1]].is_pressed = 1;
				break;
			}
			case 0xB0:
			{
				switch (u.bData[1])
				{
					case 1:
					case 64:
					{
						if ((last_controller_value < 64) && (u.bData[2] >= 64))
						{
							switch (state)
							{
								case STATE_PLAY:
								{
									key_to_edit = -1;
									state = STATE_CHOOSE_KEY_TO_EDIT:
									break;
								}
								case STATE_CHOOSE_KEY_TO_EDIT:
								{
									if (key_to_edit == -1)
									{
										state = STATE_PLAY;
									}
									else
									{
										int i;
										key_data[key_to_edit].output_channel = -1;
										key_data[key_to_edit].output_note = -1;
										key_data[key_to_edit].output_velocity = -1;
										for (i = 0; i < 12; i++) key_data[key_to_edit].pattern[i] = 0;
										state = STATE_CHOOSE_OUTPUT_NOTE:
									}

									break;
								}
								case STATE_CHOOSE_OUTPUT_NOTE:
								{
									if (key_data[key_to_edit].output_channel == -1)
									{
										state = STATE_PLAY;
									}
									else
									{
										state = STATE_CHOOSE_PATTERN:
									}

									break;
								}
								case STATE_CHOOSE_PATTERN_STEPS:
								{
									state = PLAY:
									break;
								}
							}
						}

						last_controller_value = u.bData[2];
						break;
					}
					default:
					{
						break;
					}
				}

				break;
			}
			default:
			{
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
	int i, j;
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
		else
		{
			printf("Usage: %s [--in <n>] [--out <n>]\n", argv[0]);
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

	for (i = 0; i < 128; i++)
	{
		key_data[i].output_channel = -1;
		key_data[i].output_note = -1;
		key_data[i].output_velocity = -1;
		for (j = 0; j < 12; j++) key_data[i].pattern[j] = 0;
		key_data[i].is_pressed = 0;
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	midiInStart(midi_in);

	beat_start_time = now();

	while (1)
	{
		now = now();
		beat_duration = 60 / tempo;

		if (state == STATE_PLAY)
		{
			for (i = 0; i < 128; i++)
			{
				if (key_data[i].number_of_steps_in_pattern > 0)
				{
				}
			}
		}

		Sleep(CLOCK_RATE);
	}

	return 0;
}


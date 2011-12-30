
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_PULSES 100

int pulse_is_on = 0;
int note_velocity[16][128];
HMIDIIN midi_in;
HMIDIOUT midi_out;

void usage(char *progname)
{
	printf("Usage: %s [--in <n>] [--out <n>] [--pulse <time before pulse> <length of pulse> <time after pulse>] ...\n", progname);
	exit(1);
}

void send_note_on(int channel, int note, int velocity)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x90 | channel;
	u.bData[1] = note;
	u.bData[2] = velocity;
	u.bData[3] = 0x00;

	midiOutShortMsg(midi_out, u.dwData);
}

void send_note_off(int channel, int note)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x80 | channel;
	u.bData[1] = note;
	u.bData[2] = 0x00;
	u.bData[3] = 0x00;

	midiOutShortMsg(midi_out, u.dwData);
}

void note_on_handler(int channel, int note, int velocity)
{
	if (pulse_is_on)
	{
		if (note_velocity[channel][note] > 0)
		{
			send_note_off(channel, note);
		}

		send_note_on(channel, note, velocity);
	}

	note_velocity[channel][note] = velocity;
}

void note_off_handler(int channel, int note)
{
	if (pulse_is_on && (note_velocity[channel][note] > 0))
	{
		send_note_off(channel, note);
	}

	note_velocity[channel][note] = 0;
}

void pulse_on_handler(void)
{
	int channel, note;

	for (channel = 0; channel < 16; channel++)
	{
		for (note = 0; note < 128; note++)
		{
			if (note_velocity[channel][note] > 0)
			{
				send_note_on(channel, note, note_velocity[channel][note]);
			}
		}
	}

	pulse_is_on = 1;
}

void pulse_off_handler(void)
{
	int channel, note;

	for (channel = 0; channel < 16; channel++)
	{
		for (note = 0; note < 128; note++)
		{
			if (note_velocity[channel][note] > 0)
			{
				send_note_off(channel, note);
			}
		}
	}

	pulse_is_on = 0;
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
			{
				note_off_handler(u.bData[0] & 0x0F, u.bData[1]);
				break;
			}
			case 0x90:
			{
				if (u.bData[2] == 0x00)
				{
					note_off_handler(u.bData[0] & 0x0F, u.bData[1]);
				}
				else
				{
					note_on_handler(u.bData[0] & 0x0F, u.bData[1], u.bData[2]);
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
	int i, channel, note, pulse_number;
	int midi_in_number = 0;
	int midi_out_number = MIDI_MAPPER;
	int time_before_pulse[MAX_PULSES];
	int pulse_length[MAX_PULSES];
	int time_after_pulse[MAX_PULSES];
	int number_of_pulses = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &midi_in_number) != 1) usage(argv[0]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &midi_out_number) != 1) usage(argv[0]);
		}
		else if (strcmp(argv[i], "--pulse") == 0)
		{
			if (number_of_pulses == MAX_PULSES)
			{
				printf("Too many pulses.\n");
				return 1;
			}

			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &(time_before_pulse[number_of_pulses])) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &(pulse_length[number_of_pulses])) != 1) usage(argv[0]);
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &(time_after_pulse[number_of_pulses])) != 1) usage(argv[0]);
			number_of_pulses++;
		}
		else
		{
			usage(argv[0]);
		}
	}

	for (channel = 0; channel < 16; channel++)
	{
		for (note = 0; note < 128; note++)
		{
			note_velocity[channel][note] = 0;
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

	while (1)
	{
		for (pulse_number = 0; pulse_number < number_of_pulses; pulse_number++)
		{
			if (time_before_pulse[pulse_number] > 0) Sleep(time_before_pulse[pulse_number]);
			pulse_on_handler();
			if (pulse_length[pulse_number] > 0) Sleep(pulse_length[pulse_number]);
			pulse_off_handler();
			if (time_after_pulse[pulse_number] > 0) Sleep(time_after_pulse[pulse_number]);
		}
	}

	return 0;
}


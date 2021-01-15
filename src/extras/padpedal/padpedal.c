
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef enum
{
	NOTE_STATE_OFF,
	NOTE_STATE_ON,
	NOTE_STATE_SUSTAINED
}
NoteState_t;

int sustain = 0;
NoteState_t note_state[128];
int note_velocity[128];
int pad_note;
HMIDIIN primary_midi_in;
HMIDIIN pad_midi_in;
HMIDIOUT primary_midi_out;
HMIDIOUT pad_midi_out;
int primary_input_channel = 0;
int pad_input_channel = 0;
int pad_controller_number = 0x42;
int primary_output_channel = 0;
int pad_output_channel = 1;

void usage(char *progname)
{
	printf("Usage: %s [--in <port> <channel>] [--pad-in <port> <channel> <controller>] [--out <port> <channel>] [--pad-out <port> <channel>]\n", progname);
	exit(1);
}

void send_note_on(HMIDIOUT midi_out, int channel, int note, int velocity)
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

void send_note_off(HMIDIOUT midi_out, int channel, int note)
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

void send_sustain_on(HMIDIOUT midi_out, int channel)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xB0 | channel;
	u.bData[1] = 0x40;
	u.bData[2] = 0x7F;
	u.bData[3] = 0x00;

	midiOutShortMsg(midi_out, u.dwData);
}

void send_sustain_off(HMIDIOUT midi_out, int channel)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0xB0 | channel;
	u.bData[1] = 0x40;
	u.bData[2] = 0x00;
	u.bData[3] = 0x00;

	midiOutShortMsg(midi_out, u.dwData);
}

void note_on_handler(int note, int velocity)
{
	note_state[note] = NOTE_STATE_ON;
	note_velocity[note] = velocity;
	send_note_on(primary_midi_out, primary_output_channel, note, velocity);
}

void note_off_handler(int note)
{
	if (sustain)
	{
		note_state[note] = NOTE_STATE_SUSTAINED;
	}
	else
	{
		note_state[note] = NOTE_STATE_OFF;
		note_velocity[note] = 0;
	}

	send_note_off(primary_midi_out, primary_output_channel, note);
}

void sustain_on_handler(void)
{
	sustain = 1;
	send_sustain_on(primary_midi_out, primary_output_channel);
}

void sustain_off_handler(void)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_state[note] == NOTE_STATE_SUSTAINED)
		{
			note_state[note] = NOTE_STATE_OFF;
			note_velocity[note] = 0;
		}
	}

	sustain = 0;
	send_sustain_off(primary_midi_out, primary_output_channel);
}

void pad_on_handler(void)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_state[note] != NOTE_STATE_OFF)
		{
			pad_note = note;
			send_note_on(pad_midi_out, pad_output_channel, note, note_velocity[note]);
			break;
		}
	}
}

void pad_off_handler(void)
{
	send_note_off(pad_midi_out, pad_output_channel, pad_note);
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

		if ((midi_in == primary_midi_in) && ((u.bData[0] & 0x0F) == primary_input_channel))
		{
			switch (u.bData[0] & 0xF0)
			{
				case 0x80:
				{
					note_off_handler(u.bData[1]);
					break;
				}
				case 0x90:
				{
					if (u.bData[2] == 0x00)
					{
						note_off_handler(u.bData[1]);
					}
					else
					{
						note_on_handler(u.bData[1], u.bData[2]);
					}

					break;
				}
				case 0xB0:
				{
					if (u.bData[1] == 0x40)
					{
						if (u.bData[2] >= 0x40)
						{
							sustain_on_handler();
						}
						else
						{
							sustain_off_handler();
						}
					}
					else
					{
						midiOutShortMsg(primary_midi_out, u.dwData);
					}

					break;
				}
				case 0xA0:
				case 0xC0:
				case 0xD0:
				case 0xE0:
				{
					midiOutShortMsg(primary_midi_out, u.dwData);
					break;
				}
			}
		}
		else if ((midi_in == pad_midi_in) && (u.bData[0] == (0xB0 | pad_input_channel)) && (u.bData[1] == pad_controller_number))
		{
			if (u.bData[2] >= 0x40)
			{
				pad_on_handler();
			}
			else
			{
				pad_off_handler();
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	midiInStop(primary_midi_in);
	midiInClose(primary_midi_in);

	if (pad_midi_in != primary_midi_in)
	{
		midiInStop(pad_midi_in);
		midiInClose(pad_midi_in);
	}

	midiOutClose(primary_midi_out);

	if (pad_midi_out != primary_midi_out)
	{
		midiOutClose(pad_midi_out);
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	int i;
	int note;
	int primary_midi_in_number = 0;
	int pad_midi_in_number = -1;
	int primary_midi_out_number = MIDI_MAPPER;
	int pad_midi_out_number = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &primary_midi_in_number);
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &primary_input_channel);
		}
		else if (strcmp(argv[i], "--pad-in") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &pad_midi_in_number);
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &pad_input_channel);
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &pad_controller_number);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &primary_midi_out_number);
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &primary_output_channel);
		}
		else if (strcmp(argv[i], "--pad-out") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &pad_midi_out_number);
			if (++i >= argc) usage(argv[0]);
			sscanf(argv[i], "%d", &pad_output_channel);
		}
		else
		{
			usage(argv[0]);
		}
	}

	for (note = 0; note < 128; note++)
	{
		note_state[note] = NOTE_STATE_OFF;
		note_velocity[note] = 0;
	}

	if (pad_midi_in_number == -1) pad_midi_in_number = primary_midi_in_number;
	if (pad_midi_out_number == -1) pad_midi_out_number = primary_midi_out_number;

	if (midiInOpen(&primary_midi_in, primary_midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", primary_midi_in_number);
		return 1;
	}

	if (pad_midi_in_number == primary_midi_in_number)
	{
		pad_midi_in = primary_midi_in;
	}
	else
	{
		if (midiInOpen(&pad_midi_in, pad_midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		{
			printf("Cannot open MIDI input port #%d.\n", pad_midi_in_number);
			return 1;
		}
	}

	if (midiOutOpen(&primary_midi_out, primary_midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", primary_midi_out_number);
		return 1;
	}

	if (pad_midi_out_number == primary_midi_out_number)
	{
		pad_midi_out = primary_midi_out;
	}
	else
	{
		if (midiOutOpen(&pad_midi_out, pad_midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
		{
			printf("Cannot open MIDI output port #%d.\n", pad_midi_out_number);
			return 1;
		}
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	midiInStart(primary_midi_in);
	if (pad_midi_in != primary_midi_in) midiInStart(pad_midi_in);

	Sleep(INFINITE);
	return 0;
}


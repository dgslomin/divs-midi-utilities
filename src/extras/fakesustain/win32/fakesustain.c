
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

int sustain[16];
NoteState_t note_state[16][128];
HMIDIIN midi_in;
HMIDIOUT midi_out;

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
	switch (note_state[channel][note])
	{
		case NOTE_STATE_ON:
		{
			send_note_off(channel, note);
			send_note_on(channel, note, velocity);
			break;
		}
		case NOTE_STATE_OFF:
		{
			send_note_on(channel, note, velocity);
			note_state[channel][note] = NOTE_STATE_ON;
			break;
		}
		case NOTE_STATE_SUSTAINED:
		{
			send_note_off(channel, note);
			send_note_on(channel, note, velocity);
			note_state[channel][note] = NOTE_STATE_ON;
			break;
		}
	}
}

void note_off_handler(int channel, int note)
{
	if (note_state[channel][note] == NOTE_STATE_ON)
	{
		if (sustain[channel])
		{
			note_state[channel][note] = NOTE_STATE_SUSTAINED;
		}
		else
		{
			send_note_off(channel, note);
			note_state[channel][note] = NOTE_STATE_OFF;
		}
	}
}

void sustain_on_handler(int channel)
{
	sustain[channel] = 1;
}

void sustain_off_handler(int channel)
{
	int note;

	for (note = 0; note < 128; note++)
	{
		if (note_state[channel][note] == NOTE_STATE_SUSTAINED)
		{
			send_note_off(channel, note);
			note_state[channel][note] = NOTE_STATE_OFF;
		}
	}

	sustain[channel] = 0;
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
			case 0xB0:
			{
				if (u.bData[1] == 0x40)
				{
					if (u.bData[2] >= 0x40)
					{
						sustain_on_handler(u.bData[0] & 0x0F);
					}
					else
					{
						sustain_off_handler(u.bData[0] & 0x0F);
					}
				}
				else
				{
					midiOutShortMsg(midi_out, u.dwData);
				}

				break;
			}
			case 0xA0:
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
	int i, channel, note;
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

	for (channel = 0; channel < 16; channel++)
	{
		sustain[channel] = 0;

		for (note = 0; note < 128; note++)
		{
			note_state[channel][note] = NOTE_STATE_OFF;
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

	Sleep(INFINITE);
	return 0;
}


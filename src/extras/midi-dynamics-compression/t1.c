
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void usage(char *progname)
{
	printf("Usage: %s [ --out <port> ]\n", progname);
	exit(1);
}

void note_on(HMIDIOUT midi_out, int channel, int pitch, int velocity)
{
	union
	{
		DWORD dwData;
		BYTE bData[4];
	}
	u;

	u.bData[0] = 0x90 | channel;
	u.bData[1] = pitch;
	u.bData[2] = velocity;
	u.bData[3] = 0x00;
	midiOutShortMsg(midi_out, u.dwData);
}

int main(int argc, char **argv)
{
	int i;
	int midi_out_number = MIDI_MAPPER;
	HMIDIOUT midi_out;
	HANDLE console_input;
	INPUT_RECORD input_record;
	DWORD records_read;
	int velocity = 64;
	int done = 0;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--out") == 0)
		{
			if (++i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &midi_out_number) != 1) usage(argv[0]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.\n", midi_out_number);
		return 1;
	}

	console_input = GetStdHandle(STD_INPUT_HANDLE);

	printf("%d\n", velocity);

	while (!done)
	{
		note_on(midi_out, 0, 60, velocity);

		if (WaitForSingleObject(console_input, 1000) == WAIT_OBJECT_0)
		{
			ReadConsoleInput(console_input, &input_record, 1, &records_read);

			if ((input_record.EventType == KEY_EVENT) && input_record.Event.KeyEvent.bKeyDown)
			{
				switch (input_record.Event.KeyEvent.wVirtualKeyCode)
				{
					case VK_ESCAPE:
					{
						done = 1;
						break;
					}
					case VK_UP:
					{
						velocity++;
						if (velocity > 127) velocity = 127;
						printf("%d\n", velocity);
						break;
					}
					case VK_DOWN:
					{
						velocity--;
						if (velocity < 0) velocity = 0;
						printf("%d\n", velocity);
						break;
					}
					case VK_PRIOR:
					{
						velocity += 10;
						if (velocity > 127) velocity = 127;
						printf("%d\n", velocity);
						break;
					}
					case VK_NEXT:
					{
						velocity -= 10;
						if (velocity < 0) velocity = 0;
						printf("%d\n", velocity);
						break;
					}
					default:
					{
						break;
					}
				}
			}
		}

		note_on(midi_out, 0, 60, 0);
	}

	midiOutClose(midi_out);
	return 0;
}


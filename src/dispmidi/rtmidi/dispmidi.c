
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midiutil.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port>\n", program_name);
	exit(1);
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (message[0] & 0xF0)
	{
		case 0x80:
		{
			printf("%d\tNote Off\t\t%d\t%d\n", message[0] & 0x0F, message[1], message[2]);
			break;
		}
		case 0x90:
		{
			printf("%d\tNote On\t\t\t%d\t%d\n", message[0] & 0x0F, message[1], message[2]);
			break;
		}
		case 0xA0:
		{
			printf("%d\tAftertouch (key)\t\t%d\t%d\n", message[0] & 0x0F, message[1], message[2]);
			break;
		}
		case 0xB0:
		{
			printf("%d\tController\t\t%d\t%d\n", message[0] & 0x0F, message[1], message[2]);
			break;
		}
		case 0xC0:
		{
			printf("%d\tProgram\t\t%d\n", message[0] & 0x0F, message[1]);
			break;
		}
		case 0xD0:
		{
			printf("%d\tAftertouch (channel)\t%d\n", message[0] & 0x0F, message[1]);
			break;
		}
		case 0xE0:
		{
			printf("%d\tPitch Bend\t\t%x\n", message[0] & 0x0F, (int)(message[2]) << 7 | (int)(message[1]));
			break;
		}
		default:
		{
			break;
		}
	}
}

int main(int argc, char **argv)
{
	RtMidiInPtr midi_in = NULL;
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
		}
		else if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("dispmidi", argv[i], "in", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_in == NULL) usage(argv[0]);
	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);
	return 0;
}


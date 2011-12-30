
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

struct midi_in_list_node
{
	int midi_in_number;
	HMIDIIN midi_in;
	struct midi_in_list_node *next_midi_in_list_node;
};

struct midi_in_list_node *first_midi_in_list_node = NULL;

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
{
	int midi_in_number;

	midiInGetID(midi_in, &midi_in_number);

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
				printf(" %3d | %02X %02X %02X | Note Off         | Channel %2d | Pitch    %3d | Velocity %3d\n", midi_in_number, u.bData[0], u.bData[1], u.bData[2], u.bData[0] & 0x0F, u.bData[1], u.bData[2]);
				break;
			}
			case 0x90:
			{
				printf(" %3d | %02X %02X %02X | Note On          | Channel %2d | Pitch    %3d | Velocity %3d\n", midi_in_number, u.bData[0], u.bData[1], u.bData[2], u.bData[0] & 0x0F, u.bData[1], u.bData[2]);
				break;
			}
			case 0xA0:
			{
				printf(" %3d | %02X %02X %02X | Key Pressure     | Channel %2d | Pitch    %3d | Amount   %3d\n", midi_in_number, u.bData[0], u.bData[1], u.bData[2], u.bData[0] & 0x0F, u.bData[1], u.bData[2]);
				break;
			}
			case 0xB0:
			{
				printf(" %3d | %02X %02X %02X | Control Change   | Channel %2d | Number   %3d | Value    %3d\n", midi_in_number, u.bData[0], u.bData[1], u.bData[2], u.bData[0] & 0x0F, u.bData[1], u.bData[2]);
				break;
			}
			case 0xC0:
			{
				printf(" %3d | %02X %02X    | Program Change   | Channel %2d | Number   %3d |\n", midi_in_number, u.bData[0], u.bData[1], u.bData[0] & 0x0F, u.bData[1]);
				break;
			}
			case 0xD0:
			{
				printf(" %3d | %02X %02X    | Channel Pressure | Channel %2d | Amount   %3d |\n", midi_in_number, u.bData[0], u.bData[1], u.bData[0] & 0x0F, u.bData[1]);
				break;
			}
			case 0xE0:
			{
				printf(" %3d | %02X %02X %02X | Pitch Wheel      | Channel %2d | Amount %5d |\n", midi_in_number, u.bData[0], u.bData[1], u.bData[2], u.bData[0] & 0x0F, (int)(u.bData[2]) << 7 | (int)(u.bData[1]));
				break;
			}
		}

		fflush(stdout);
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	struct midi_in_list_node *midi_in_list_node, *next_midi_in_list_node;

	for (midi_in_list_node = first_midi_in_list_node; midi_in_list_node != NULL; midi_in_list_node = next_midi_in_list_node)
	{
		next_midi_in_list_node = midi_in_list_node->next_midi_in_list_node;
		midiInStop(midi_in_list_node->midi_in);
		midiInClose(midi_in_list_node->midi_in);
		free(midi_in_list_node);
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	int i;
	struct midi_in_list_node *midi_in_list_node;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			int midi_in_number = 0;

			if (++i >= argc) break;
			if (sscanf(argv[i], "%d", &midi_in_number) != 1) break;

			midi_in_list_node = (struct midi_in_list_node *)(malloc(sizeof (struct midi_in_list_node)));
			midi_in_list_node->midi_in_number = midi_in_number;
			midi_in_list_node->next_midi_in_list_node = first_midi_in_list_node;
			first_midi_in_list_node = midi_in_list_node;
		}
		else
		{
			printf("Usage: %s [--in <n>] ...\n", argv[0]);
			return 1;
		}
	}

	if (first_midi_in_list_node == NULL)
	{
		first_midi_in_list_node = (struct midi_in_list_node *)(malloc(sizeof (struct midi_in_list_node)));
		first_midi_in_list_node->midi_in_number = 0;
		first_midi_in_list_node->next_midi_in_list_node = NULL;
	}

	for (midi_in_list_node = first_midi_in_list_node; midi_in_list_node != NULL; midi_in_list_node = midi_in_list_node->next_midi_in_list_node)
	{
		if (midiInOpen(&(midi_in_list_node->midi_in), midi_in_list_node->midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		{
			printf("Cannot open MIDI input port #%d.\n", midi_in_list_node->midi_in_number);
			return 1;
		}
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	for (midi_in_list_node = first_midi_in_list_node; midi_in_list_node != NULL; midi_in_list_node = midi_in_list_node->next_midi_in_list_node)
	{
		midiInStart(midi_in_list_node->midi_in);
	}

	Sleep(INFINITE);
	return 0;
}


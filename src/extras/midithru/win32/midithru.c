
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>

struct MidiInListNode
{
	HMIDIIN midi_in;
	struct MidiInListNode *next;
};

typedef struct MidiInListNode *MidiInListNode_t;

struct MidiOutListNode
{
	HMIDIOUT midi_out;
	struct MidiOutListNode *next;
};

typedef struct MidiOutListNode *MidiOutListNode_t;

MidiInListNode_t midi_in_list_head;
MidiOutListNode_t midi_out_list_head;

void usage(char *argv0)
{
	printf("Usage: %s [ --in <input device id> ... ] [ --out <output device id> ... ]\n", argv0);
	exit(1);
}

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
{
	if (msg_type == MIM_DATA)
	{
		MidiOutListNode_t midi_out_list_node;

		union
		{
			DWORD dwData;
			BYTE bData[4];
		}
		u;

		u.dwData = midi_msg;

		if ((u.bData[0] & 0xF0) != 0xF0)
		{
			for (midi_out_list_node = midi_out_list_head; midi_out_list_node != NULL; midi_out_list_node = midi_out_list_node->next) midiOutShortMsg(midi_out_list_node->midi_out, midi_msg);
		}
	}
}

void add_midi_in(UINT device_id)
{
	HRESULT result;
	MidiInListNode_t midi_in_list_node;

	midi_in_list_node = (MidiInListNode_t)(malloc(sizeof(struct MidiInListNode)));
	midi_in_list_node->next = midi_in_list_head;
	midi_in_list_head = midi_in_list_node;

	if ((result = midiInOpen(&(midi_in_list_node->midi_in), device_id, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
	{
		switch (result)
		{
			case MMSYSERR_ALLOCATED:
			{
				printf("Input device %d is already in use.\n", device_id);
				break;
			}
			case MMSYSERR_BADDEVICEID:
			{
				printf("Input device %d does not exist.\n", device_id);
				break;
			}
			default:
			{
				printf("Internal error.\n");
				break;
			}
		}

		exit(1);
	}

	midiInStart(midi_in_list_node->midi_in);
}

void add_midi_out(UINT device_id)
{
	HRESULT result;
	MidiOutListNode_t midi_out_list_node;

	midi_out_list_node = (MidiOutListNode_t)(malloc(sizeof(struct MidiOutListNode)));
	midi_out_list_node->next = midi_out_list_head;
	midi_out_list_head = midi_out_list_node;

	if ((result = midiOutOpen(&(midi_out_list_node->midi_out), device_id, (DWORD)(NULL), (DWORD)(NULL), CALLBACK_NULL)) != MMSYSERR_NOERROR)
	{
		switch (result)
		{
			case MMSYSERR_ALLOCATED:
			{
				printf("Output device %d is already in use.\n", device_id);
				break;
			}
			case MMSYSERR_BADDEVICEID:
			{
				printf("Output device %d does not exist.\n", device_id);
				break;
			}
			default:
			{
				printf("Internal error.\n");
				break;
			}
		}

		exit(1);
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	MidiInListNode_t midi_in_list_node;
	MidiOutListNode_t midi_out_list_node;

	for (midi_in_list_node = midi_in_list_head; midi_in_list_node != NULL; midi_in_list_node = midi_in_list_node->next)
	{
		midiInStop(midi_in_list_node->midi_in);
		midiInClose(midi_in_list_node->midi_in);
	}

	for (midi_out_list_node = midi_out_list_head; midi_out_list_node != NULL; midi_out_list_node = midi_out_list_node->next)
	{
		midiOutClose(midi_out_list_node->midi_out);
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	int i = 1;
	int midi_ins = 0;
	int midi_outs = 0;

	midi_in_list_head = NULL;
	midi_out_list_head = NULL;

	SetConsoleCtrlHandler(control_handler, TRUE);

	while (i < argc)
	{
		if ((strcmp(argv[i], "--in") == 0) || (strcmp(argv[i], "--from") == 0))
		{
			UINT device_id;

			i++;
			if (i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &device_id) != 1) usage(argv[0]);
			i++;

			add_midi_in(device_id);
			midi_ins++;
		}
		else if ((strcmp(argv[i], "--out") == 0) || (strcmp(argv[i], "--to") == 0))
		{
			UINT device_id;

			i++;
			if (i >= argc) usage(argv[0]);
			if (sscanf(argv[i], "%d", &device_id) != 1) usage(argv[0]);
			i++;

			add_midi_out(device_id);
			midi_outs++;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_ins == 0) add_midi_in(0);
	if (midi_outs == 0) add_midi_out(MIDI_MAPPER);

	Sleep(INFINITE);
	return 0;
}


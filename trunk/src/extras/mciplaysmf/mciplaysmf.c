
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef enum
{
	MCI_PLAY_SMF_RESULT_OK = 0,
	MCI_PLAY_SMF_RESULT_CANNOT_OPEN_FILE,
	MCI_PLAY_SMF_RESULT_CANNOT_OPEN_PORT
}
MciPlaySmfResult_t;

static MciPlaySmfResult_t mci_play_smf(char *filename, int midi_out_number)
{
	MCI_OPEN_PARMS mci_open_parms;
	MCI_SEQ_SET_PARMS mci_seq_set_parms;

	mci_open_parms.lpstrDeviceType = "Sequencer";
	mci_open_parms.lpstrElementName = filename;

	if (mciSendCommand((MCIDEVICEID)(NULL), MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)(&mci_open_parms)) != 0)
	{
		return MCI_PLAY_SMF_RESULT_CANNOT_OPEN_FILE;
	}

	if (midi_out_number >= 0)
	{
		mci_seq_set_parms.dwPort = midi_out_number;

		if (mciSendCommand(mci_open_parms.wDeviceID, MCI_SET, MCI_SEQ_SET_PORT, (DWORD)(&mci_seq_set_parms)) != 0) 
		{
			return MCI_PLAY_SMF_RESULT_CANNOT_OPEN_PORT;
		}
	}

	mciSendCommand(mci_open_parms.wDeviceID, MCI_PLAY, MCI_WAIT, (DWORD)(NULL));
	mciSendCommand(mci_open_parms.wDeviceID, MCI_STOP, MCI_WAIT, (DWORD)(NULL));
	mciSendCommand(mci_open_parms.wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD)(NULL));
	return MCI_PLAY_SMF_RESULT_OK;
}

static void usage(char *program_name)
{
	printf("Usage:  %s [ --out <device id> ] [ -- ] <filename.mid>\n", program_name);
	exit(1);
}

static int get_midi_out_number(char *name)
{
	int midi_out_number, maximum_midi_out_number;

	for (midi_out_number = 0, maximum_midi_out_number = midiOutGetNumDevs(); midi_out_number < maximum_midi_out_number; midi_out_number++)
	{
		MIDIOUTCAPS midi_out_caps;
		midiOutGetDevCaps(midi_out_number, &midi_out_caps, sizeof(midi_out_caps));
		if (strcmp(midi_out_caps.szPname, name) == 0) return midi_out_number;
	}

	if (sscanf(name, "%d", &midi_out_number) == 1) return midi_out_number;
	return -1;
}

int main(int argc, char **argv)
{
	int i;
	int midi_out_number = MIDI_MAPPER;
	char *filename = NULL;
	int double_dash = 0;

	for (i = 1; i < argc; i++)
	{
		if (!double_dash && ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "/help") == 0) || (strcmp(argv[i], "/h") == 0) || (strcmp(argv[i], "/?") == 0)))
		{
			usage(argv[0]);
		}
		else if (!double_dash && (strcmp(argv[i], "--out") == 0))
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out_number = get_midi_out_number(argv[i])) < 0)
			{
				printf("Error:  No such MIDI output device \"%s\".\n", argv[i]);
				return 1;
			}
		}
		else if (!double_dash && (strcmp(argv[i], "--") == 0))
		{
			double_dash = 1;
		}
		else if (filename == NULL)
		{
			filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (filename == NULL)
	{
		usage(argv[0]);
	}

	switch (mci_play_smf(filename, midi_out_number))
	{
		case MCI_PLAY_SMF_RESULT_OK:
		{
			break;
		}
		case MCI_PLAY_SMF_RESULT_CANNOT_OPEN_FILE:
		{
			printf("Error:  Cannot open \"%s\".\n", filename);
			break;
		}
		case MCI_PLAY_SMF_RESULT_CANNOT_OPEN_PORT:
		{
			printf("Error:  Cannot open MIDI output port #%d.\n", midi_out_number);
			break;
		}
	}

	return 0;
}


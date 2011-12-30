
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

static int done = 0;

static void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT message_type, DWORD user_data, DWORD midi_message, DWORD param2)
{
	if (message_type == MIM_LONGDATA)
	{
		done = 1;
	}
}

static void CALLBACK midi_out_handler(HMIDIOUT midi_out, UINT message_type, DWORD user_data, DWORD midi_message, DWORD param2)
{
	done = 1;
}

static BOOL WINAPI interrupt_handler(DWORD control_type)
{
	done = 1;
	return FALSE;
}

static int load(char *filename, unsigned char **buffer_p)
{
	struct _stat file_info;
	int buffer_size;
	FILE *in;

	if (_stat(filename, &file_info) < 0)
	{
		*buffer_p = NULL;
		return 0;
	}

	buffer_size = file_info.st_size;
	*buffer_p = (unsigned char *)(malloc(buffer_size));
	in = fopen(filename, "rb");
	fread(*buffer_p, 1, buffer_size, in);
	fclose(in);
	return buffer_size;
}

static void save(char *filename, unsigned char *buffer, int buffer_size)
{
	FILE *out = fopen(filename, "wb");
	fwrite(buffer, 1, buffer_size, out);
	fclose(out);
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s ( --in <n> [ --buffer <bytes> ] | --out <n> ) <filename.syx>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int midi_in_number = -1;
	int midi_out_number = -1;
	int buffer_size = 1024 * 1024;
	char *filename = NULL;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_in_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_out_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--buffer") == 0)
		{
			if (++i == argc) usage(argv[0]);
			buffer_size = atoi(argv[i]);
		}
		else
		{
			filename = argv[i];
		}
	}

	if ((midi_out_number < 0) && (midi_in_number < 0) || (filename == NULL))
	{
		usage(argv[0]);
	}

	if (midi_in_number > 0)
	{
		HMIDIIN midi_in;
		unsigned char *buffer;
		MIDIHDR midi_message_header;

		fprintf(stderr, "Debug:  Running in sysex save mode.\n");

		if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		{
			fprintf(stderr, "Error:  Cannot open MIDI input port #%d.\n", midi_in_number);
			return 1;
		}

		buffer = (unsigned char *)(malloc(buffer_size));
		midi_message_header.lpData = buffer;
		midi_message_header.dwBufferLength = buffer_size;
		midi_message_header.dwFlags = 0;
		fprintf(stderr, "Debug:  About to start MIDI input.\n");
		midiInStart(midi_in);
		fprintf(stderr, "Debug:  Started MIDI input, about to prepare message header.\n");
		midiInPrepareHeader(midi_in, &midi_message_header, sizeof(midi_message_header));
		fprintf(stderr, "Debug:  Prepared message header, about to add buffer.\n");
		midiInAddBuffer(midi_in, &midi_message_header, sizeof(midi_message_header));
		fprintf(stderr, "Debug:  Added buffer, about the setup interrupt handler.\n");
		SetConsoleCtrlHandler(interrupt_handler, TRUE);
		fprintf(stderr, "Debug:  Setup interrupt handler, entering loop.\n");

		while (!done)
		{
			Sleep(250);
		}

		fprintf(stderr, "Debug:  Finished loop.\n");

		if (midi_message_header.dwBytesRecorded > 0)
		{
			fprintf(stderr, "Debug:  About to save.\n");
			save(filename, midi_message_header.lpData, midi_message_header.dwBytesRecorded);
			fprintf(stderr, "Debug:  Saved.\n");
		}

		fprintf(stderr, "Debug:  About to unprepare message header.\n");
		midiInUnprepareHeader(midi_in, &midi_message_header, sizeof(midi_message_header));
		fprintf(stderr, "Debug:  Unprepared message header, about to stop MIDI input.\n");
		free(buffer);
		midiInStop(midi_in);
		fprintf(stderr, "Debug:  Stopped MIDI input, about to close MIDI input.\n");
		midiInClose(midi_in);
		fprintf(stderr, "Debug:  Closed MIDI input.\n");
	}
	else
	{
		HMIDIOUT midi_out;
		unsigned char *buffer;
		int buffer_size;
		MIDIHDR midi_message_header;

		if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
		{
			fprintf(stderr, "Error:  Cannot open MIDI output port #%d.\n", midi_out_number);
			return 1;
		}

		if ((buffer_size = load(filename, &buffer)) == 0)
		{
			fprintf(stderr, "Error:  Cannot load \"%s\".\n", filename);
			return 1;
		}

		midi_message_header.lpData = buffer;
		midi_message_header.dwBufferLength = buffer_size;
		midi_message_header.dwFlags = 0;
		midiOutPrepareHeader(midi_out, &midi_message_header, sizeof(midi_message_header));
		midiOutLongMsg(midi_out, &midi_message_header, sizeof(midi_message_header));

		while (!done)
		{
			Sleep(250);
		}

		midiOutUnprepareHeader(midi_out, &midi_message_header, sizeof(midi_message_header));
		free(buffer);
		midiOutClose(midi_out);
	}

	return 0;
}


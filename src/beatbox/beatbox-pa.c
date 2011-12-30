
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <portaudio.h>
#include "beatbox.h"

Beatbox_t beatbox;
PortAudioStream *audio_stream;
HMIDIIN midi_in;

static void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
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

		if ((u.bData[0] & 0xF0) == 0x90)
		{
			Beatbox_noteOn(beatbox, u.bData[1], u.bData[2]);
		}
	}
}

static int compute_audio_handler(void *input_p, void *output_p, unsigned long frames, PaTimestamp out_time, void *user_data)
{
	float *output_data = (float *)(output_p);

	Beatbox_computeAudio(beatbox, output_data, frames);

	return 0;
}

BOOL WINAPI control_handler(DWORD control_type)
{
	midiInStop(midi_in);
	midiInClose(midi_in);

	Pa_StopStream(audio_stream);
	Pa_CloseStream(audio_stream);
	Pa_Terminate();

	Beatbox_free(beatbox);

	return FALSE;
}

void usage(char *program_name)
{
	printf("Usage:  %s [ --in <n> ] [ --voices <n> ] ( [ --config <filename> ] ) ...\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	int midi_in_number = 0;
	int number_of_voices = 64;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--voices") == 0)
		{
			if (++i == argc) usage(argv[0]);
			number_of_voices = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
		}
		else if (strcmp(argv[i], "--config") == 0)
		{
			if (++i == argc) usage(argv[0]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	SetConsoleCtrlHandler(control_handler, TRUE);

	beatbox = Beatbox_new(number_of_voices);

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--voices") == 0)
		{
			if (++i == argc) usage(argv[0]);
		}
		else if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_in_number = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--config") == 0)
		{
			if (++i == argc) usage(argv[0]);
			Beatbox_loadConfig(beatbox, argv[i]);
		}
		else
		{
			usage(argv[0]);
		}
	}

	Pa_Initialize();
	Pa_OpenDefaultStream(&audio_stream, 0, 2, paFloat32, 44100, 256, 0, compute_audio_handler, NULL);
	Pa_StartStream(audio_stream);

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	midiInStart(midi_in);

	Sleep(INFINITE);
	return 0;
}


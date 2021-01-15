
/*
 * Brainstorm - a dictatation machine for MIDI
 * Copyright 2000, 2002, 2004 David G. Slomin
 * Brainstorm is Free Software provided under terms of the BSD license
 * In addition to this Windows version, there is also a Posix version by the same author
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <simplealarm.h>
#include <midifile.h>

enum
{
	MAX_NUMBER_OF_MIDI_EVENTS = 1048576
};

struct MidiEvent
{
	DWORD timestamp;
	DWORD message;
};

typedef struct MidiEvent MidiEvent;

HMIDIIN midi_in;
int timeout;
int extra_time;
char *prefix;
int verbose;
SimpleAlarm_t alarm;
int number_of_midi_events;
MidiEvent midi_events[MAX_NUMBER_OF_MIDI_EVENTS];

static void saveMidiEventsToFile(void)
{
	if (number_of_midi_events > 0)
	{
		SYSTEMTIME current_time;
		char filename[1024];
		MidiFile_t midi_file;
		MidiFileTrack_t track;
		int midi_event_number;

		midiInStop(midi_in);

		GetLocalTime(&current_time);
		sprintf(filename, "%s%d%02d%02d%02d%02d%02d.mid", prefix, current_time.wYear, current_time.wMonth, current_time.wDay, current_time.wHour, current_time.wMinute, current_time.wSecond);

		midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);

		track = MidiFile_createTrack(midi_file); /* conductor track */

		{
			unsigned char time_signature[] = { 4, 2, 24, 8 }; /* 4/4 */
			MidiFileTrack_createMetaEvent(track, 0, 0x58, 4, time_signature);
		}

		{
			unsigned char key_signature[] = { 0, 0 }; /* C major */
			MidiFileTrack_createMetaEvent(track, 0, 0x59, 2, key_signature);
		}

		{
			unsigned char tempo[] = { 0x09, 0x27, 0xC0 }; /* 100 BPM */
			MidiFileTrack_createMetaEvent(track, 0, 0x51, 3, tempo);
		}

		track = MidiFile_createTrack(midi_file); /* main track */

		for (midi_event_number = 0; midi_event_number < number_of_midi_events; midi_event_number++)
		{
			union
			{
				DWORD dwData;
				unsigned char bData[4];
			}
			u;

			u.dwData = midi_events[midi_event_number].message;

			switch (u.bData[0] & 0xF0)
			{
				case 0x80:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int note = u.bData[1];
					int velocity = u.bData[2];
					MidiFileTrack_createNoteOffEvent(track, tick, channel, note, velocity);
					break;
				}
				case 0x90:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int note = u.bData[1];
					int velocity = u.bData[2];
					MidiFileTrack_createNoteOnEvent(track, tick, channel, note, velocity);
					break;
				}
				case 0xA0:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int note = u.bData[1];
					int amount = u.bData[2];
					MidiFileTrack_createKeyPressureEvent(track, tick, channel, note, amount);
					break;
				}
				case 0xB0:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int number = u.bData[1];
					int value = u.bData[2];
					MidiFileTrack_createControlChangeEvent(track, tick, channel, number, value);
					break;
				}
				case 0xC0:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int number = u.bData[1];
					MidiFileTrack_createProgramChangeEvent(track, tick, channel, number);
					break;
				}
				case 0xD0:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int amount = u.bData[1];
					MidiFileTrack_createChannelPressureEvent(track, tick, channel, amount);
					break;
				}
				case 0xE0:
				{
					long tick = (midi_events[midi_event_number].timestamp - midi_events[0].timestamp) * 8 / 5;
					int channel = u.bData[0] & 0x0F;
					int value = (u.bData[1] << 7) | u.bData[2];
					MidiFileTrack_createPitchWheelEvent(track, tick, channel, value);
					break;
				}
				default:
				{
					/* ignore everything else */
					break;
				}
			}
		}

		if (extra_time > 0)
		{
#if 0
			MidiFileTrack_setEndTick(track, MidiFileTrack_getEndTick(track) + ((long)(extra_time) * 8000 / 5));
#else
			/* This is an undesirable hack, but unfortunately, the clean method above doesn't seem to do the trick. */
			MidiFileTrack_createNoteOnEvent(track, MidiFileTrack_getEndTick(track) + ((long)(extra_time) * 8000 / 5), 0, 0, 0);
#endif
		}

		if (MidiFile_save(midi_file, filename) != 0)
		{
			printf("Error:  Cannot write to %s\n", filename);
			exit(1);
		}

		if (verbose)
		{
			printf("Info:  Wrote %d events to \"%s\".\n", number_of_midi_events, filename);
		}

		number_of_midi_events = 0;

		midiInStart(midi_in);
	}
}

static void usage(char *argv0)
{
	printf("Usage:  %s [ --in <device id> ] [ --timeout <seconds> ] [ --extra-time <seconds> ] [ --prefix <filename prefix> ] [ --verbose ]\n", argv0);
	exit(1);
}

static void handleAlarm(void *user_data)
{
	saveMidiEventsToFile();
}

static void CALLBACK handleMidiMessage(HMIDIIN midi_in, UINT message_type, DWORD user_data, DWORD param1, DWORD param2)
{
	DWORD timestamp = timeGetTime();

	if (message_type == MIM_DATA)
	{
		union
		{
			DWORD dwData;
			unsigned char bData[4];
		}
		u;

		u.dwData = param1;

		if ((u.bData[0] & 0xF0) != 0xF0)
		{
			SimpleAlarm_set(alarm, timeout * 1000, handleAlarm, NULL);

			midi_events[number_of_midi_events].timestamp = timestamp;
			midi_events[number_of_midi_events].message = param1;
			number_of_midi_events++;

			if (number_of_midi_events == MAX_NUMBER_OF_MIDI_EVENTS)
			{
				saveMidiEventsToFile();
			}
		}
	}
}

BOOL WINAPI handleConsoleCtrl(DWORD control_type)
{
	saveMidiEventsToFile();
	midiInStop(midi_in);
	midiInClose(midi_in);
	return FALSE;
}

int get_midi_in_device_id(char *name)
{
	int id, max;

	for (id = 0, max = midiInGetNumDevs(); id < max; id++)
	{
		MIDIINCAPS midi_in_caps;
		midiInGetDevCaps(id, &midi_in_caps, sizeof(midi_in_caps));
		if (strcmp(midi_in_caps.szPname, name) == 0) return id;
	}

	if (sscanf(name, "%d", &id) == 1) return id;
	return -1;
}

int main(int argc, char **argv)
{
	int i;
	int device_id = 0;
	HRESULT result;
	HANDLE console_input;

	timeout = 5;
	extra_time = 0;
	prefix = "brainstorm-";
	verbose = 0;
	alarm = SimpleAlarm_new();
	number_of_midi_events = 0;

	for (i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "--in") == 0) || (strcmp(argv[i], "--from") == 0))
		{
			if (++i == argc) usage(argv[0]);

			if ((device_id = get_midi_in_device_id(argv[i])) < 0)
			{
				printf("Error:  No such input device \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--timeout") == 0)
		{
			if (++i == argc) usage(argv[0]);
			if ((sscanf(argv[i], "%d", &timeout) != 1) || (timeout < 1)) usage(argv[0]);
		}
		else if (strcmp(argv[i], "--extra-time") == 0)
		{
			if (++i == argc) usage(argv[0]);
			if ((sscanf(argv[i], "%d", &extra_time) != 1) || (extra_time < 0)) usage(argv[0]);
		}
		else if (strcmp(argv[i], "--prefix") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefix = _strdup(argv[i]);
		}
		else if (strcmp(argv[i], "--verbose") == 0)
		{
			verbose = 1;
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((result = midiInOpen(&midi_in, device_id, (DWORD)(handleMidiMessage), (DWORD)(NULL), CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
	{
		switch (result)
		{
			case MMSYSERR_ALLOCATED:
			{
				printf("Error:  Input device %d is already in use.\n", device_id);
				break;
			}
			case MMSYSERR_BADDEVICEID:
			{
				printf("Error:  Input device %d does not exist.\n", device_id);
				break;
			}
			default:
			{
				printf("Error:  Internal error.\n");
				break;
			}
		}

		return 1;
	}

	SetConsoleCtrlHandler(handleConsoleCtrl, TRUE);

	midiInStart(midi_in);

	console_input = GetStdHandle(STD_INPUT_HANDLE);

	while (1)
	{
		INPUT_RECORD input_record;
		DWORD records_read;

		ReadConsoleInput(console_input, &input_record, 1, &records_read);

		if ((input_record.EventType == KEY_EVENT) && input_record.Event.KeyEvent.bKeyDown)
		{
			if (input_record.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
			{
				break;
			}
			else if (input_record.Event.KeyEvent.wVirtualKeyCode == VK_SPACE)
			{
				SimpleAlarm_set(alarm, timeout * 1000, handleAlarm, NULL);
				saveMidiEventsToFile();
			}
		}
	}

	saveMidiEventsToFile();
	midiInStop(midi_in);
	midiInClose(midi_in);
	return 0;
}


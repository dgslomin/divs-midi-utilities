
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <expat.h>
#include <midiutil-common.h>
#include <midiutil-system.h>
#include <midiutil-rtmidi.h>
#include <midiutil-expat.h>

static RtMidiInPtr midi_in = NULL;
static RtMidiOutPtr midi_out = NULL;
static int transposition = 0;
static signed char notemap[128];

static void usage(char *program_name)
{
	fprintf(stderr, "Usage:  %s --in <port> --out <port> [ --transpose <n> ] [ --map <filename> ]\n", program_name);
	exit(1);
}

static void handle_xml_start_element(void *user_data, const XML_Char *name, const XML_Char **attributes)
{
	if (strcmp(name, "map") == 0)
	{
		int i, from = -1, to = -1;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "from") == 0)
			{
				from = MidiUtil_getNoteNumberFromName((char *)(attributes[i + 1]));
			}
			else if (strcmp(attributes[i], "to") == 0)
			{
				to = MidiUtil_getNoteNumberFromName((char *)(attributes[i + 1]));
			}
		}

		if ((from >= 0) && (to >= 0))
		{
			notemap[from] = to;
		}
	}
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	switch (MidiUtilMessage_getType(message))
	{
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_OFF:
		{
			unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF];
			int new_note = notemap[MidiUtilNoteOffMessage_getNote(message)];
			if (new_note < 0) break;
			new_note += transposition;
			if ((new_note < 0) || (new_note >= 128)) break;
			MidiUtilMessage_setNoteOff(new_message, MidiUtilNoteOffMessage_getChannel(message), new_note, MidiUtilNoteOffMessage_getVelocity(message));
			rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_OFF);
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_NOTE_ON:
		{
			unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_NOTE_ON];
			int new_note = notemap[MidiUtilNoteOnMessage_getNote(message)];
			if (new_note < 0) break;
			new_note += transposition;
			MidiUtilMessage_setNoteOn(new_message, MidiUtilNoteOnMessage_getChannel(message), new_note, MidiUtilNoteOnMessage_getVelocity(message));
			rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_NOTE_ON);
			break;
		}
		case MIDI_UTIL_MESSAGE_TYPE_KEY_PRESSURE:
		{
			unsigned char new_message[MIDI_UTIL_MESSAGE_SIZE_KEY_PRESSURE];
			int new_note = notemap[MidiUtilKeyPressureMessage_getNote(message)];
			if (new_note < 0) break;
			new_note += transposition;
			if ((new_note < 0) || (new_note >= 128)) break;
			MidiUtilMessage_setKeyPressure(new_message, MidiUtilKeyPressureMessage_getChannel(message), new_note, MidiUtilKeyPressureMessage_getAmount(message));
			rtmidi_out_send_message(midi_out, new_message, MIDI_UTIL_MESSAGE_SIZE_KEY_PRESSURE);
			break;
		}
		default:
		{
			rtmidi_out_send_message(midi_out, message, message_size);
			break;
		}
	}
}

static void handle_exit(void *user_data)
{
	rtmidi_close_port(midi_in);
	rtmidi_close_port(midi_out);
}

int main(int argc, char **argv)
{
	int i;

	for (i = 0; i < 128; i++) notemap[i] = -1;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_in = rtmidi_open_in_port("notemap", argv[i], "notemap", handle_midi_message, NULL)) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);

			if ((midi_out = rtmidi_open_out_port("notemap", argv[i], "notemap")) == NULL)
			{
				fprintf(stderr, "Error:  Cannot open MIDI output port \"%s\".\n", argv[i]);
				exit(1);
			}
		}
		else if (strcmp(argv[i], "--transpose") == 0)
		{
			if (++i == argc) usage(argv[0]);
			transposition = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "--map") == 0)
		{
			XML_Parser xml_parser = XML_ParserCreate(NULL);
			char error_message[1024];
			if (++i == argc) usage(argv[0]);
			XML_SetStartElementHandler(xml_parser, handle_xml_start_element);

			if (XML_ParseFile(xml_parser, argv[i], error_message, 1024) < 0)
			{
				printf("Error:  %s\n", error_message);
				exit(1);
			}

			XML_ParserFree(xml_parser);
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((midi_in == NULL) || (midi_out == NULL)) usage(argv[0]);
	MidiUtil_waitForExit(handle_exit, NULL);
	return 0;
}


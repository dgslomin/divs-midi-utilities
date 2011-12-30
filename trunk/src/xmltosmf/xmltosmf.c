
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <midifile.h>

MidiFile_t midi_file;
MidiFileTrack_t track;

static void usage(char *program_name)
{
	printf("Usage: %s <filename>.xml <filename>.mid\n", program_name);
	exit(1);
}

static void xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **attributes)
{
	if (strcmp(name, "MidiFile") == 0)
	{
		int i;
		MidiFileDivisionType_t division_type = MIDI_FILE_DIVISION_TYPE_INVALID;
		int resolution = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "DivisionType") == 0)
			{
				if (strcmp(attributes[i + 1], "PPQ") == 0)
				{
					division_type = MIDI_FILE_DIVISION_TYPE_PPQ;
				}
				else if (strcmp(attributes[i + 1], "SMPTE24") == 0)
				{
					division_type = MIDI_FILE_DIVISION_TYPE_SMPTE24;
				}
				else if (strcmp(attributes[i + 1], "SMPTE25") == 0)
				{
					division_type = MIDI_FILE_DIVISION_TYPE_SMPTE25;
				}
				else if (strcmp(attributes[i + 1], "SMPTE30DROP") == 0)
				{
					division_type = MIDI_FILE_DIVISION_TYPE_SMPTE30DROP;
				}
				else if (strcmp(attributes[i + 1], "SMPTE30") == 0)
				{
					division_type = MIDI_FILE_DIVISION_TYPE_SMPTE30;
				}
			}
			else if (strcmp(attributes[i], "Resolution") == 0)
			{
				sscanf(attributes[i + 1], "%d", &resolution);
			}
		}

		midi_file = MidiFile_new(1, division_type, resolution);
	}
	else if (strcmp(name, "Track") == 0)
	{
		int i, end_tick = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "EndTick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &end_tick);
			}
		}

		track = MidiFile_createTrack(midi_file);
		MidiFileTrack_setEndTick(track, end_tick);
	}
	else if (strcmp(name, "NoteOffEvent") == 0)
	{
		int i, tick = 0, channel = 0, note = 0, velocity = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Note") == 0)
			{
				sscanf(attributes[i + 1], "%d", &note);
			}
			else if (strcmp(attributes[i], "Velocity") == 0)
			{
				sscanf(attributes[i + 1], "%d", &velocity);
			}
		}

		MidiFileTrack_createNoteOffEvent(track, tick, channel, note, velocity);
	}
	else if (strcmp(name, "NoteOnEvent") == 0)
	{
		int i, tick = 0, channel = 0, note = 0, velocity = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Note") == 0)
			{
				sscanf(attributes[i + 1], "%d", &note);
			}
			else if (strcmp(attributes[i], "Velocity") == 0)
			{
				sscanf(attributes[i + 1], "%d", &velocity);
			}
		}

		MidiFileTrack_createNoteOnEvent(track, tick, channel, note, velocity);
	}
	else if (strcmp(name, "KeyPressureEvent") == 0)
	{
		int i, tick = 0, channel = 0, note = 0, amount = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Note") == 0)
			{
				sscanf(attributes[i + 1], "%d", &note);
			}
			else if (strcmp(attributes[i], "Amount") == 0)
			{
				sscanf(attributes[i + 1], "%d", &amount);
			}
		}

		MidiFileTrack_createKeyPressureEvent(track, tick, channel, note, amount);
	}
	else if (strcmp(name, "ControlChangeEvent") == 0)
	{
		int i, tick = 0, channel = 0, number = 0, value = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Number") == 0)
			{
				sscanf(attributes[i + 1], "%d", &number);
			}
			else if (strcmp(attributes[i], "Value") == 0)
			{
				sscanf(attributes[i + 1], "%d", &value);
			}
		}

		MidiFileTrack_createControlChangeEvent(track, tick, channel, number, value);
	}
	else if (strcmp(name, "ProgramChangeEvent") == 0)
	{
		int i, tick = 0, channel = 0, number = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Number") == 0)
			{
				sscanf(attributes[i + 1], "%d", &number);
			}
		}

		MidiFileTrack_createProgramChangeEvent(track, tick, channel, number);
	}
	else if (strcmp(name, "ChannelPressureEvent") == 0)
	{
		int i, tick = 0, channel = 0, amount = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Amount") == 0)
			{
				sscanf(attributes[i + 1], "%d", &amount);
			}
		}

		MidiFileTrack_createChannelPressureEvent(track, tick, channel, amount);
	}
	else if (strcmp(name, "PitchWheelEvent") == 0)
	{
		int i, tick = 0, channel = 0, value = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Channel") == 0)
			{
				sscanf(attributes[i + 1], "%d", &channel);
			}
			else if (strcmp(attributes[i], "Value") == 0)
			{
				sscanf(attributes[i + 1], "%d", &value);
			}
		}

		MidiFileTrack_createPitchWheelEvent(track, tick, channel, value);
	}
	else if (strcmp(name, "SysexEvent") == 0)
	{
		int i, tick = 0, data_length = 0;
		unsigned char *data_buffer = (unsigned char *)("");

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Data") == 0)
			{
				data_length = (strlen(attributes[i + 1]) + 1) / 3;

				if (data_length > 0)
				{
					int j;
					data_buffer = (unsigned char *)(malloc(data_length));

					for (j = 0; j < data_length; j++)
					{
						unsigned int data_byte;
						sscanf(attributes[i + 1] + (j * 3), "%x", &data_byte);
						data_buffer[j] = (unsigned char)(data_byte);
					}
				}
			}
		}

		MidiFileTrack_createSysexEvent(track, tick, data_length, data_buffer);
	}
	else if (strcmp(name, "MetaEvent") == 0)
	{
		int i, tick = 0, number = 0, data_length = 0;
		unsigned char *data_buffer = (unsigned char *)("");

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "Tick") == 0)
			{
				sscanf(attributes[i + 1], "%d", &tick);
			}
			else if (strcmp(attributes[i], "Number") == 0)
			{
				sscanf(attributes[i + 1], "%d", &number);
			}
			else if (strcmp(attributes[i], "Data") == 0)
			{
				data_length = (strlen(attributes[i + 1]) + 1) / 3;

				if (data_length > 0)
				{
					int j;
					data_buffer = (unsigned char *)(malloc(data_length));

					for (j = 0; j < data_length; j++)
					{
						unsigned int data_byte;
						sscanf(attributes[i + 1] + (j * 3), "%x", &data_byte);
						data_buffer[j] = (unsigned char)(data_byte);
					}
				}
			}
		}

		MidiFileTrack_createMetaEvent(track, tick, number, data_length, data_buffer);
	}
}

static int XML_ParseFile(XML_Parser parser, char *filename)
{
	FILE *f;
	void *buffer;
	int bytes_read;

	if ((parser == NULL) || (filename == NULL)) return -1;
	if ((f = fopen(filename, "rb")) == NULL) return -1;

	do
	{
		buffer = XML_GetBuffer(parser, 1024);
		bytes_read = fread(buffer, 1, 1024, f);
		XML_ParseBuffer(parser, bytes_read, (bytes_read == 0));
	}
	while (bytes_read > 0);

	fclose(f);
	return 0;
}

int main(int argc, char **argv)
{
	char *input_filename, *output_filename;
	XML_Parser xml_parser;

	if (argc != 3) usage(argv[0]);
	input_filename = argv[1];
	output_filename = argv[2];
	xml_parser = XML_ParserCreate(NULL);
	XML_SetStartElementHandler(xml_parser, xml_start_element_handler);

	if (XML_ParseFile(xml_parser, input_filename) < 0)
	{
		printf("Error: Cannot load XML file \"%s\".\n", input_filename);
		return 1;
	}

	MidiFile_save(midi_file, output_filename);
	return 0;
}


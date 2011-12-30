
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <expat.h>

int midi_in_number = 0, midi_out_number = MIDI_MAPPER;
signed char notemap[128];
HMIDIIN midi_in;
HMIDIOUT midi_out;

void xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **attributes)
{
	if (strcmp(name, "notemap") == 0)
	{
		int i, use_null_for_default = 0;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "default") == 0)
			{
				if (strcmp(attributes[i + 1], "null") == 0) use_null_for_default = 1;
			}
			else if (strcmp(attributes[i], "midi-in") == 0)
			{
				sscanf(attributes[i + 1], "%d", &midi_in_number);
			}
			else if (strcmp(attributes[i], "midi-out") == 0)
			{
				sscanf(attributes[i + 1], "%d", &midi_out_number);
			}
		}

		if (use_null_for_default)
		{
			for (i = 0; i < 128; i++) notemap[i] = -1;
		}
		else
		{
			for (i = 0; i < 128; i++) notemap[i] = i;
		}
	}
	else if (strcmp(name, "mapping") == 0)
	{
		int i, from = -1, to = -1;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "from") == 0)
			{
				sscanf(attributes[i + 1], "%d", &from);
			}
			else if (strcmp(attributes[i], "to") == 0)
			{
				sscanf(attributes[i + 1], "%d", &to);
			}
		}

		if ((from >= 0) && (to >= 0))
		{
			notemap[from] = to;
		}
	}
	else if (strcmp(name, "null-mapping") == 0)
	{
		int i, from = -1;

		for (i = 0; attributes[i] != NULL; i += 2)
		{
			if (strcmp(attributes[i], "from") == 0)
			{
				sscanf(attributes[i + 1], "%d", &from);
			}
		}

		if (from >= 0)
		{
			notemap[from] = -1;
		}
	}
}

int XML_ParseFile(XML_Parser parser, char *filename)
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

void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
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

		switch (u.bData[0] & 0xF0)
		{
			case 0x80:
			case 0x90:
			{
				if (notemap[u.bData[1]] < 0) return;
				u.bData[1] = (BYTE)(notemap[u.bData[1]]);
				midiOutShortMsg(midi_out, u.dwData);
				break;
			}
			case 0xA0:
			case 0xB0:
			case 0xC0:
			case 0xD0:
			case 0xE0:
			{
				midiOutShortMsg(midi_out, u.dwData);
				break;
			}
		}
	}
}

BOOL WINAPI control_handler(DWORD control_type)
{
	midiInStop(midi_in);
	midiInClose(midi_in);
	midiOutClose(midi_out);
	return FALSE;
}

int main(int argc, char **argv)
{
	XML_Parser xml_parser;

	if (argc != 2)
	{
		printf("Usage: %s <filename>.xml\n", argv[0]);
		return 1;
	}

	xml_parser = XML_ParserCreate(NULL);
	XML_SetStartElementHandler(xml_parser, xml_start_element_handler);

	if (XML_ParseFile(xml_parser, argv[1]) < 0)
	{
		printf("Cannot load config file \"%s\".\n", argv[1]);
		return 1;
	}

	if (midiInOpen(&midi_in, midi_in_number, (DWORD)(midi_in_handler), (DWORD)(NULL), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI input port #%d.\n", midi_in_number);
		return 1;
	}

	if (midiOutOpen(&midi_out, midi_out_number, 0, 0, 0) != MMSYSERR_NOERROR)
	{
		printf("Cannot open MIDI output port #%d.", midi_out_number);
		return 1;
	}

	SetConsoleCtrlHandler(control_handler, TRUE);
	midiInStart(midi_in);
	Sleep(INFINITE);
	return 0;
}


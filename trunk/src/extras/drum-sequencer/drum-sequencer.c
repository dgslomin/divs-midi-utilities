
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <expat.h>
#include <simplealarm.h>
#include "drum-sequencer.h"

struct DrumSequencer
{
	char *filename;
	XML_Parser parser;
	int number_of_beats;
	char *trigger_port_name;
	HMIDIIN midi_in;
	int trigger_channel;
	int number_of_ports;
	Port_t *ports;
	int number_of_instruments;
	Instrument_t *instruments;
	int number_of_patterns;
	Pattern_t *patterns;
	Trigger_t triggers[128];
	SimpleAlarm_t alarm;
	int current_tempo;
	Pattern_t current_pattern;
	int current_step;
	int shutdown;
};

struct Port
{
	char *name;
	HMIDIOUT midi_out;
};

struct Instrument
{
	char *name;
	Port_t port;
	int channel;
	int note;
	int velocity;
};

struct Pattern
{
	char *name;
	int number_of_steps;
	int number_of_lines;
	Line_t *lines;
};

struct Line
{
	char *instrument_name;
	Instrument_t instrument;
	int number_of_events;
	int *events;
};

struct Trigger
{
	char *pattern_name;
	Pattern_t pattern;
};

static DrumSequencer_t main_sequencer = NULL; /* Global because of Windows' horrendous console control handler mechanism */

static int least_common_multiple(int a, int b)
{
	int i, lcm = 1;

	for (i = 0; (i < a) || (i < b); i++)
	{
		if ((((a % i) == 0) && ((b % i) != 0)) || (((a % i) != 0) && ((b % i) == 0)))
		{
			lcm *= i;
		}
	}

	return lcm;
}

static void xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **attributes)
{
	DrumSequencer_t sequencer = (DrumSequencer_t)(user_data);

	if (strcmp((char *)(name), "drum-sequencer") == 0)
	{
		int i;
		int sequencer_number_of_beats = -1;
		char *sequencer_trigger_port_name = NULL;
		int sequencer_trigger_channel = -1;

		for (i = 0; attributes[i] != NULL; i+= 2)
		{
			if (strcmp((char *)(attributes[i]), "beats") == 0)
			{
				sequencer_number_of_beats = atoi((char *)(attributes[i + 1]));
			}
			else if (strcmp((char *)(attributes[i]), "trigger-port") == 0)
			{
				sequencer_trigger_port_name = (char *)(attributes[i + 1]);
			}
			else if (strcmp((char *)(attributes[i]), "trigger-channel") == 0)
			{
				sequencer_trigger_channel = atoi((char *)(attributes[i + 1]));
			}
		}

		if (sequencer_number_of_beats > 0)
		{
			DrumSequencer_setNumberOfBeats(sequencer, sequencer_number_of_beats);
		}

		if ((sequencer_trigger_port_name != NULL) && (sequencer_trigger_channel >= 1) && (sequencer_trigger_channel <= 16))
		{
			DrumSequencer_setTriggerPortAndChannel(sequencer, sequencer_trigger_port_name, sequencer_trigger_channel);
		}
	}
	else if (strcmp((char *)(name), "port") == 0)
	{
		int i;
		char *port_name = NULL;

		for (i = 0; attributes[i] != NULL; i+= 2)
		{
			if (strcmp((char *)(attributes[i]), "name") == 0)
			{
				port_name = (char *)(attributes[i + 1]);
			}
		}

		if (port_name == NULL)
		{
			printf("Error:  You must specify a name for the port at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else
		{
			DrumSequencer_addPort(sequencer, port_name);
		}
	}
	else if (strcmp((char *)(name), "instrument") == 0)
	{
		if (sequencer->number_of_ports == 0)
		{
			printf("Error:  You must specify the instrument at \"%s\" line %d inside of a port.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else
		{
			int i;
			char *instrument_name = NULL;
			int instrument_channel = -1;
			int instrument_note = -1;
			int instrument_velocity = -1;

			for (i = 0; attributes[i] != NULL; i+= 2)
			{
				if (strcmp((char *)(attributes[i]), "name") == 0)
				{
					instrument_name = (char *)(attributes[i + 1]);
				}
				else if (strcmp((char *)(attributes[i]), "channel") == 0)
				{
					instrument_channel = atoi((char *)(attributes[i + 1]));
				}
				else if (strcmp((char *)(attributes[i]), "note") == 0)
				{
					instrument_note = atoi((char *)(attributes[i + 1]));
				}
				else if (strcmp((char *)(attributes[i]), "velocity") == 0)
				{
					instrument_velocity = atoi((char *)(attributes[i + 1]));
				}
			}

			if (instrument_name == NULL)
			{
				printf("Error:  You must specify a name for the instrument \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
				exit(1);
			}
			else if ((instrument_channel < 1) || (instrument_channel > 16))
			{
				printf("Error:  You must specify a channel for the instrument at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
				exit(1);
			}
			else if ((instrument_note < 1) || (instrument_note > 128))
			{
				printf("Error:  You must specify a note for the instrument at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
				exit(1);
			}
			else if ((instrument_velocity < 1) || (instrument_velocity > 128))
			{
				printf("Error:  You must specify a velocity for the instrument at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
				exit(1);
			}
			else
			{
				DrumSequencer_addInstrument(sequencer, instrument_name, instrument_channel, instrument_note, instrument_velocity);
			}
		}
	}
	else if (strcmp((char *)(name), "pattern") == 0)
	{
		int i;
		char *pattern_name = NULL;
		int pattern_number_of_steps = -1;

		for (i = 0; attributes[i] != NULL; i+= 2)
		{
			if (strcmp((char *)(attributes[i]), "name") == 0)
			{
				pattern_name = (char *)(attributes[i + 1]);
			}
			else if (strcmp((char *)(attributes[i]), "steps") == 0)
			{
				pattern_number_of_steps = atoi((char *)(attributes[i + 1]));
			}
		}

		if (pattern_name == NULL)
		{
			printf("Error:  You must specify a name for the pattern at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else if (pattern_number_of_steps <= 0)
		{
			printf("Error:  You must specify a number of steps for the pattern at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else
		{
			DrumSequencer_addPattern(sequencer, pattern_name, pattern_number_of_steps);
		}
	}
	else if (strcmp((char *)(name), "line") == 0)
	{
		if (sequencer->number_of_patterns == 0)
		{
			printf("Error:  You must specify the line at \"%s\" line %d inside of a pattern.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else
		{
			int i;
			char *line_instrument_name = NULL;

			for (i = 0; attributes[i] != NULL; i+= 2)
			{
				if (strcmp((char *)(attributes[i]), "instrument") == 0)
				{
					line_instrument_name = (char *)(attributes[i + 1]);
				}
			}

			if (line_instrument_name == NULL)
			{
				printf("Error:  You must specify an instrument for the line at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
				exit(1);
			}
			else
			{
				DrumSequencer_addLine(sequencer, line_instrument_name);
			}
		}
	}
	else if (strcmp((char *)(name), "trigger") == 0)
	{
		int i;
		int trigger_note = -1;
		char *trigger_pattern_name = NULL;

		for (i = 0; attributes[i] != NULL; i+= 2)
		{
			if (strcmp((char *)(attributes[i]), "note") == 0)
			{
				trigger_note = atoi((char *)(attributes[i + 1]));
			}
			else if (strcmp((char *)(attributes[i]), "pattern") == 0)
			{
				trigger_pattern_name = (char *)(attributes[i + 1]);
			}
		}

		if ((trigger_note < 0) || (trigger_note >= 128))
		{
			printf("Error:  You must specify a note for the trigger at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else if (trigger_pattern_name == NULL)
		{
			printf("Error:  You must specify a pattern for the trigger at \"%s\" line %d.\n", sequencer->filename, XML_GetCurrentLineNumber(sequencer->parser));
			exit(1);
		}
		else
		{
			DrumSequencer_addTrigger(sequencer, trigger_note, trigger_pattern_name);
		}
	}
}

static void xml_character_data_handler(void *user_data, const XML_Char *text, int length)
{
	DrumSequencer_t sequencer = (DrumSequencer_t)(user_data);
	int i;
	int is_all_space = 1;

	for (i = 0; i < length; i++)
	{
		if (! isspace((char)(text[i])))
		{
			is_all_space = 0;
			break;
		}
	}

	if (! is_all_space)
	{
		for (i = 0; i < length; i++)
		{
			if (text[i] == '0')
			{
				DrumSequencer_addEvent(sequencer, 0);
			}
			else if (text[i] == '1')
			{
				DrumSequencer_addEvent(sequencer, 1);
			}
			else
			{
				/* any other character can be used as a separator, which helps the user but is meaningless to the program */
			}
		}
	}
}

static void CALLBACK midi_in_handler(HMIDIIN midi_in, UINT msg_type, DWORD user_data, DWORD midi_msg, DWORD param2)
{
	DrumSequencer_t sequencer = (DrumSequencer_t)(user_data);
	if (msg_type == MIM_DATA)
	{
		union
		{
			DWORD dwData;
			BYTE bData[4];
		}
		u;

		u.dwData = midi_msg;

		if (u.bData[0] == (0x90 | sequencer->trigger_channel))
		{
			printf("debug:  got a trigger\n");

			if (sequencer->current_pattern != NULL)
			{
				DrumSequencer_cancelNextStep(sequencer);
			}

			sequencer->current_pattern = sequencer->triggers[u.bData[1]]->pattern;
			/* TODO: compute current step for new pattern */
			/* TODO: add logic for time elapsed since last step */

			if (sequencer->current_pattern != NULL)
			{
				DrumSequencer_scheduleNextStep(sequencer);
			}
		}
		else if ((u.bData[0] == (0xB0 | sequencer->trigger_channel)) && (u.bData[1] == 1))
		{
			if (sequencer->current_pattern != NULL)
			{
				DrumSequencer_cancelNextStep(sequencer);
			}

			sequencer->current_tempo = u.bData[2];
			/* TODO: add logic for time elapsed since last step */

			if (sequencer->current_pattern != NULL)
			{
				DrumSequencer_scheduleNextStep(sequencer);
			}
		}
	}
}

static void alarm_handler(void *user_data)
{
	DrumSequencer_t sequencer = (DrumSequencer_t)(user_data);
	int i;

	for (i = 0; i < sequencer->current_pattern->number_of_lines; i++)
	{
		Line_t line = sequencer->current_pattern->lines[i];

		if (line->events[sequencer->current_step])
		{
			union
			{
				DWORD dwData;
				BYTE bData[4];
			}
			u;

			u.bData[0] = 0x90 | line->instrument->channel;
			u.bData[1] = line->instrument->note;
			u.bData[2] = line->instrument->velocity;
			u.bData[3] = 0;

			midiOutShortMsg(line->instrument->port->midi_out, u.dwData);
		}
	}

	sequencer->current_step = (sequencer->current_step + 1) % sequencer->current_pattern->number_of_steps;
	DrumSequencer_scheduleNextStep(sequencer);
}

BOOL WINAPI console_control_handler(DWORD control_type)
{
	if (main_sequencer != NULL)
	{
		DrumSequencer_stop(main_sequencer);
	}

	return TRUE;
}

DrumSequencer_t DrumSequencer_new(void)
{
	DrumSequencer_t sequencer = (DrumSequencer_t)(malloc(sizeof(struct DrumSequencer)));
	int i;

	sequencer->filename = NULL;
	sequencer->parser = NULL;
	sequencer->number_of_beats = -1;
	sequencer->trigger_port_name = NULL;
	sequencer->midi_in = 0;
	sequencer->trigger_channel = -1;
	sequencer->number_of_ports = 0;
	sequencer->ports = NULL;
	sequencer->number_of_instruments = 0;
	sequencer->instruments = NULL;
	sequencer->number_of_patterns = 0;
	sequencer->patterns = NULL;

	for (i = 0; i < 128; i++)
	{
		sequencer->triggers[i] = NULL;
	}

	sequencer->alarm = SimpleAlarm_new();
	sequencer->current_tempo = 120;
	sequencer->current_pattern = NULL;
	sequencer->current_step = 0;

	return sequencer;
}

void DrumSequencer_free(DrumSequencer_t sequencer)
{
	int i;

	for (i = 0; i < sequencer->number_of_ports; i++)
	{
		Port_t port = sequencer->ports[i];
		free(port->name);
		midiOutClose(port->midi_out);
		free(port);
	}

	if (sequencer->ports != NULL)
	{
		free(sequencer->ports);
	}

	for (i = 0; i < sequencer->number_of_instruments; i++)
	{
		Instrument_t instrument = sequencer->instruments[i];
		free(instrument->name);
		free(instrument);
	}

	if (sequencer->instruments != NULL)
	{
		free(sequencer->instruments);
	}

	for (i = 0; i < sequencer->number_of_patterns; i++)
	{
		Pattern_t pattern = sequencer->patterns[i];
		int j;

		for (j = 0; j < pattern->number_of_lines; j++)
		{
			Line_t line = pattern->lines[j];
			free(line->instrument_name);

			if (line->events != NULL)
			{
				free(line->events);
			}

			free(line);
		}

		if (pattern->lines != NULL)
		{
			free(pattern->lines);
		}

		free(pattern->name);
		free(pattern);
	}

	if (sequencer->patterns != NULL)
	{
		free(sequencer->patterns);
	}

	for (i = 0; i < 128; i++)
	{
		Trigger_t trigger = sequencer->triggers[i];

		if (trigger != NULL)
		{
			free(trigger->pattern_name);
			free(trigger);
		}
	}

	SimpleAlarm_free(sequencer->alarm);
	free(sequencer);
}

void DrumSequencer_setNumberOfBeats(DrumSequencer_t sequencer, int number_of_beats)
{
	sequencer->number_of_beats = number_of_beats;
}

void DrumSequencer_setTriggerPortAndChannel(DrumSequencer_t sequencer, char *port_name, int channel)
{
	sequencer->trigger_port_name = strdup(port_name);
	sequencer->trigger_channel = channel - 1;
}

void DrumSequencer_addPort(DrumSequencer_t sequencer, char *name)
{
	Port_t port = (Port_t)(malloc(sizeof(struct Port)));
	port->name = strdup(name);
	port->midi_out = 0;
	sequencer->ports = (Port_t *)(realloc(sequencer->ports, ++(sequencer->number_of_ports) * sizeof(Port_t)));
	sequencer->ports[sequencer->number_of_ports - 1] = port;
}

void DrumSequencer_addInstrument(DrumSequencer_t sequencer, char *name, int channel, int note, int velocity)
{
	Port_t port;
	Instrument_t instrument;

	if (sequencer->number_of_ports == 0)
	{
		printf("Error:  You cannot add an instrument without a port.\n");
		exit(1);
	}

	port = sequencer->ports[sequencer->number_of_ports - 1];
	instrument = (Instrument_t)(malloc(sizeof(struct Instrument)));
	instrument->name = strdup(name);
	instrument->port = port;
	instrument->channel = channel - 1;
	instrument->note = note - 1;
	instrument->velocity = velocity - 1;
	sequencer->instruments = (Instrument_t *)(realloc(sequencer->instruments, ++(sequencer->number_of_instruments) * sizeof(Instrument_t)));
	sequencer->instruments[sequencer->number_of_instruments - 1] = instrument;
}

void DrumSequencer_addPattern(DrumSequencer_t sequencer, char *name, int number_of_steps)
{
	Pattern_t pattern = (Pattern_t)(malloc(sizeof(struct Pattern)));
	pattern->name = strdup(name);
	pattern->number_of_steps = number_of_steps;
	pattern->number_of_lines = 0;
	pattern->lines = NULL;
	sequencer->patterns = (Pattern_t *)(realloc(sequencer->patterns, ++(sequencer->number_of_patterns) * sizeof(Pattern_t)));
	sequencer->patterns[sequencer->number_of_patterns - 1] = pattern;
}

void DrumSequencer_addLine(DrumSequencer_t sequencer, char *instrument_name)
{
	Pattern_t pattern;
	Line_t line;

	if (sequencer->number_of_patterns == 0)
	{
		printf("Error:  You cannot add a line without a pattern.\n");
		exit(1);
	}

	pattern = sequencer->patterns[sequencer->number_of_patterns - 1];
	line = (Line_t)(malloc(sizeof(struct Line)));
	line->instrument_name = strdup(instrument_name);
	line->number_of_events = 0;
	line->instrument = NULL;
	line->events = (int *)(calloc(pattern->number_of_steps, sizeof(int)));
	pattern->lines = (Line_t *)(realloc(pattern->lines, ++(pattern->number_of_lines) * sizeof(Pattern_t)));
	pattern->lines[pattern->number_of_lines - 1] = line;
}

void DrumSequencer_addEvent(DrumSequencer_t sequencer, int event)
{
	Pattern_t pattern;
	Line_t line;

	if (sequencer->number_of_patterns == 0)
	{
		printf("Error:  You cannot add an event without a pattern.\n");
		exit(1);
	}

	pattern = sequencer->patterns[sequencer->number_of_patterns - 1];

	if (pattern->number_of_lines == 0)
	{
		printf("Error:  You cannot add an event without a line.\n");
		exit(1);
	}

	line = pattern->lines[pattern->number_of_lines - 1];
	line->events[(line->number_of_events)++] = event;
}

void DrumSequencer_addTrigger(DrumSequencer_t sequencer, int note, char *pattern_name)
{
	Trigger_t trigger = (Trigger_t)(malloc(sizeof(struct Trigger)));
	trigger->pattern_name = strdup(pattern_name);
	trigger->pattern = NULL;

	if (sequencer->triggers[note] != NULL)
	{
		free(sequencer->triggers[note]->pattern_name);
		free(sequencer->triggers[note]);
	}

	sequencer->triggers[note] = trigger;
}

DrumSequencer_t DrumSequencer_load(char *filename)
{
	FILE *in;

	if ((in = fopen(filename, "rb")) == NULL)
	{
		printf("Error:  Cannot load \"%s\".\n", filename);
		exit(1);
	}
	else
	{
		DrumSequencer_t sequencer = DrumSequencer_new();
		void *buffer;
		int bytes_read;

		sequencer->filename = filename;
		sequencer->parser = XML_ParserCreate(NULL);
		XML_SetUserData(sequencer->parser, sequencer);
		XML_SetStartElementHandler(sequencer->parser, xml_start_element_handler);
		XML_SetCharacterDataHandler(sequencer->parser, xml_character_data_handler);

		do
		{
			buffer = XML_GetBuffer(sequencer->parser, 1024);
			bytes_read = fread(buffer, 1, 1024, in);
			XML_ParseBuffer(sequencer->parser, bytes_read, (bytes_read == 0));
		}
		while (bytes_read > 0);

		XML_ParserFree(sequencer->parser);
		sequencer->filename = NULL;
		sequencer->parser = NULL;
		fclose(in);

		return sequencer;
	}
}

void DrumSequencer_scheduleNextStep(DrumSequencer_t sequencer)
{
	/*
	 *              pattern     BEATS beats      minute     60000 milliseconds
	 *   1 step * ----------- * ----------- * ----------- * ------------------ = RESULT milliseconds
	 *            STEPS steps     pattern     TEMPO beats         minute
	 */

	SimpleAlarm_set(sequencer->alarm, (sequencer->number_of_beats * 60000) / (sequencer->current_pattern->number_of_steps * sequencer->current_tempo), alarm_handler, sequencer);
}

void DrumSequencer_cancelNextStep(DrumSequencer_t sequencer)
{
	SimpleAlarm_cancel(sequencer->alarm);
}

void DrumSequencer_run(DrumSequencer_t sequencer)
{
	int i;
	int number_of_ticks = 1;

	for (i = 0; i < midiInGetNumDevs(); i++)
	{
		MIDIINCAPS midi_in_caps;

		midiInGetDevCaps(i, &midi_in_caps, sizeof(midi_in_caps));

		if (strcmp(sequencer->trigger_port_name, midi_in_caps.szPname) == 0)
		{
			if (midiInOpen(&(sequencer->midi_in), i, (DWORD)(midi_in_handler), (DWORD)(sequencer), CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
			{
				printf("Error:  Cannot open MIDI input port \"%s\".\n", sequencer->trigger_port_name);
				exit(1);
			}

			break;
		}
	}

	if (i == midiInGetNumDevs())
	{
		printf("Error:  There is no MIDI input port \"%s\" on this machine.\n", sequencer->trigger_port_name);
		exit(1);
	}

	for (i = 0; i < sequencer->number_of_ports; i++)
	{
		Port_t port = sequencer->ports[i];
		int j;

		for (j = 0; j < midiOutGetNumDevs(); j++)
		{
			MIDIOUTCAPS midi_out_caps;

			midiOutGetDevCaps(j, &midi_out_caps, sizeof(midi_out_caps));

			if (strcmp(port->name, midi_out_caps.szPname) == 0)
			{
				if (midiOutOpen(&(port->midi_out), j, 0, 0, 0) != MMSYSERR_NOERROR)
				{
					printf("Error:  Cannot open MIDI output port \"%s\".\n", port->name);
					exit(1);
				}

				break;
			}
		}

		if (j == midiOutGetNumDevs())
		{
			printf("Error:  There is no MIDI output port \"%s\" on this machine.\n", port->name);
			exit(1);
		}
	}

	for (i = 0; i < sequencer->number_of_patterns; i++)
	{
		Pattern_t pattern = sequencer->patterns[i];
		int j;

		for (j = 0; j < pattern->number_of_lines; j++)
		{
			Line_t line = pattern->lines[j];
			int k;

			for (k = 0; k < sequencer->number_of_instruments; k++)
			{
				Instrument_t instrument = sequencer->instruments[k];

				if (strcmp(line->instrument_name, instrument->name) == 0)
				{
					line->instrument = instrument;
					break;
				}
			}

			if (line->instrument == NULL)
			{
				printf("Error:  You cannot make use of the instrument \"%s\" without defining it.\n", line->instrument_name);
				exit(1);
			}
		}
	}

	for (i = 0; i < sequencer->number_of_patterns; i++)
	{
		Pattern_t pattern = sequencer->patterns[i];

		number_of_ticks = least_common_multiple(number_of_ticks, pattern->number_of_steps);
	}

	sequencer->shutdown = 0;

	for (i = 0; i < 128; i++)
	{
		Trigger_t trigger = sequencer->triggers[i];

		if (trigger != NULL)
		{
			int j;

			for (j = 0; j < sequencer->number_of_patterns; j++)
			{
				Pattern_t pattern = sequencer->patterns[j];

				if (strcmp(trigger->pattern_name, pattern->name) == 0)
				{
					trigger->pattern = pattern;
					break;
				}
			}

			if (trigger->pattern == NULL)
			{
				printf("Error:  You cannot make use of the pattern \"%s\" without defining it.\n", trigger->pattern_name);
				exit(1);
			}
		}
	}

	while (! sequencer->shutdown)
	{
		Sleep(1);
	}

	midiInStop(sequencer->midi_in);
	midiInClose(sequencer->midi_in);

	for (i = 0; i < sequencer->number_of_ports; i++)
	{
		Port_t port = sequencer->ports[i];
		midiOutClose(port->midi_out);
	}
}

void DrumSequencer_stop(DrumSequencer_t sequencer)
{
	sequencer->shutdown = 1;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage:  %s <filename>\n", argv[0]);
		exit(1);
	}

	SetConsoleCtrlHandler(console_control_handler, TRUE);
	main_sequencer = DrumSequencer_load(argv[1]);
	DrumSequencer_run(main_sequencer);
	DrumSequencer_free(main_sequencer);
	return 0;
}


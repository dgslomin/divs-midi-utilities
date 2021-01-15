
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "midifile.h"
#include "reader.h"

typedef enum
{
	ARTICULATION_TYPE_NORMAL,
	ARTICULATION_TYPE_STACCATO,
	ARTICULATION_TYPE_LEGATO
}
ArticulationType_t;

MidiFileTrack_t get_track_by_number(MidiFile_t midi_file, int number)
{
	MidiFileTrack_t track = MidiFile_getFirstTrack(midi_file);
	int i = 0;

	while (1)
	{
		if (track == NULL) track = MidiFile_createTrack(midi_file);
		if (i == number) break;
		i++;
		track = MidiFileTrack_getNextTrack(track);
	}

	return track;
}

int find_previous_controller_value(MidiFileTrack_t track, int channel, int number, long time)
{
	MidiFileEvent_t event;

	for (event = MidiFileTrack_getLastEvent(track); event != NULL; event = MidiFileEvent_getPreviousEvent(event))
	{
		if ((MidiFileEvent_getTick(event) <= time) && (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) && (MidiFileControlChangeEvent_getChannel(event) == channel) && (MidiFileControlChangeEvent_getNumber(event) == number))
		{
			return MidiFileControlChangeEvent_getValue(event);
		}
	}

	return 0;
}

void create_controller_ramp(MidiFileTrack_t track, float start_time, float end_time, int channel, int number, int start_value, int end_value)
{
	int value;
	long time;
	long previous_time = -1;

	if (start_value < end_value)
	{
		for (value = start_value; value < end_value; value++)
		{
			time = (long)(start_time + ((end_time - start_time) * (value - start_value) / (end_value - start_value)));

			if (time != previous_time)
			{
				MidiFileTrack_createControlChangeEvent(track, time, channel, number, value);
				previous_time = time;
			}
		}
	}
	else if (start_value > end_value)
	{
		for (value = start_value; value > end_value; value--)
		{
			time = (long)(start_time + ((end_time - start_time) * (start_value - value) / (start_value - end_value)));

			if (time != previous_time)
			{
				MidiFileTrack_createControlChangeEvent(track, time, channel, number, value);
				previous_time = time;
			}
		}
	}
}

void create_lyric_event(MidiFileTrack_t track, long time, char *lyric)
{
	MidiFileTrack_createMetaEvent(track, time, 0x05, strlen(lyric), (unsigned char *)(lyric));
}

void create_tempo_event(MidiFile_t midi_file, long time, float tempo_in_bpm)
{
	long midi_tempo = (long)(60000000.0 / tempo_in_bpm);
	unsigned char data[3];
	data[0] = (unsigned char)((midi_tempo >> 16) & 0xFF);
	data[1] = (unsigned char)((midi_tempo >> 8) & 0xFF);
	data[2] = (unsigned char)(midi_tempo & 0xFF);
	MidiFileTrack_createMetaEvent(get_track_by_number(midi_file, 0), time, 0x51, 3, data);
}

void create_tempo_ramp(MidiFile_t midi_file, float start_time, float end_time, float start_value, float end_value)
{
	float value;
	long time;
	long previous_time = -1;

	if (start_value < end_value)
	{
		for (value = start_value; value < end_value; value += 1.0)
		{
			time = (long)(start_time + ((end_time - start_time) * (value - start_value) / (end_value - start_value)));

			if (time != previous_time)
			{
				create_tempo_event(midi_file, time, value);
				previous_time = time;
			}
		}
	}
	else if (start_value > end_value)
	{
		for (value = start_value; value > end_value; value -= 1.0)
		{
			time = (long)(start_time + ((end_time - start_time) * (start_value - value) / (start_value - end_value)));

			if (time != previous_time)
			{
				create_tempo_event(midi_file, time, value);
				previous_time = time;
			}
		}
	}
}

float find_previous_tempo(MidiFile_t midi_file, long time)
{
	MidiFileEvent_t event;

	for (event = MidiFileTrack_getLastEvent(MidiFile_getFirstTrack(midi_file)); event != NULL; event = MidiFileEvent_getPreviousEvent(event))
	{
		if ((MidiFileEvent_getTick(event) <= time) && (MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_META) && (MidiFileMetaEvent_getNumber(event) == 0x51))
		{
			unsigned char *data = MidiFileMetaEvent_getData(event);
			long midi_tempo = (((long)(data[0]) << 16) | ((long)(data[1]) << 8) | (long)(data[2]));
			float tempo_in_bpm = 60000000.0 / (float)(midi_tempo);
			return tempo_in_bpm;
		}
	}

	return 120.0;
}

void create_marker_event(MidiFile_t midi_file, long time, char *marker)
{
	MidiFileTrack_createMetaEvent(get_track_by_number(midi_file, 0), time, 0x06, strlen(marker), (unsigned char *)(marker));
}

long find_marker(MidiFile_t midi_file, char *marker)
{
	MidiFileEvent_t event;

	for (event = MidiFileTrack_getFirstEvent(MidiFile_getFirstTrack(midi_file)); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if ((MidiFileEvent_getType(event) == MIDI_FILE_EVENT_TYPE_META) && (MidiFileMetaEvent_getNumber(event) == 0x06) && (MidiFileMetaEvent_getDataLength(event) == strlen(marker)) && (strncmp((char *)(MidiFileMetaEvent_getData(event)), marker, strlen(marker)) == 0))
		{
			return MidiFileEvent_getTick(event);
		}
	}

	return -1;
}

int match_string(char **pp, char *str)
{
	while ((**pp != '\0') && (**pp == *str))
	{
		(*pp)++;
		str++;
	}

	return (*str == '\0');
}

int match_repeated_char(char **pp, char c, int *count_p)
{
	*count_p = 0;

	while (**pp == c)
	{
		(*count_p)++;
		(*pp)++;
	}

	return (*count_p > 0);
}

int match_char_from_set(char **pp, char *set, char *c_p)
{
	*c_p = '\0';

	while (*set != '\0')
	{
		if (**pp == *set)
		{
			*c_p = *set;
			(*pp)++;
			return 1;
		}

		set++;
	}

	return 0;
}

int match_number(char **pp, int *value_p)
{
	int result = 0;
	char c;
	*value_p = 0;

	while (match_char_from_set(pp, "0123456789", &c))
	{
		*value_p = (*value_p * 10) + (c - '0'); 
		result = 1;
	}

	return result;
}

int match_hex_number(char **pp, int *value_p)
{
	int result = 0;
	char c;
	*value_p = 0;

	while (1)
	{
		if (match_char_from_set(pp, "0123456789", &c))
		{
			*value_p = (*value_p * 0x10) + (c - '0'); 
			result = 1;
		}
		else if (match_char_from_set(pp, "ABCDEF", &c))
		{
			*value_p = (*value_p * 0x10) + 0xA + (c - 'A'); 
			result = 1;
		}
		else
		{
			break;
		}
	}

	return result;
}

int match_terminated_string(char **pp, char terminator, char *value)
{
	while ((**pp != '\0') && (**pp != terminator))
	{
		*(value++) = **pp;
		(*pp)++;
	}

	*value = '\0';
	return (**pp == terminator);
}

void process_mish(char *input_filename, MidiFile_t midi_file)
{
	Reader_t reader;
	int c;
	int current_note = 63;
	int current_velocity = 63;
	float current_beats_per_measure = 4.0;
	float current_time = 0.0;
	int current_channel = 0;
	int current_track = 1;
	float current_ticks_per_measure = 480.0;
	ArticulationType_t current_articulation_type;
	int current_articulation_min_length = 12;
	int current_articulation_gap_length = 12;

	if ((reader = Reader_new(input_filename)) == NULL)
	{
		printf("Cannot open input file \"%s\".\n", input_filename);
		exit(1);
	}

	while (1)
	{
		if ((c = Reader_getc(reader)) == EOF)
		{
			break;
		}
		else if (isspace(c))
		{
			continue;
		}
		else if (c == '#')
		{
			while (((c = Reader_getc(reader)) != EOF) && (c != '\n') && (c != '\r')) {}
			continue;
		}
		else if (c == '\"')
		{
			char value[1024];
			int len = 0;

			while (1)
			{
				if ((c = Reader_getc(reader)) == EOF)
				{
					printf("Syntax error at line %d of input file \"%s\" - unterminated string.\n", Reader_getLineNumber(reader), Reader_getFilename(reader));
					exit(1);
				}
				else
				{
					if (c == '\"')
					{
						break;
					}
					else if (c == '\\')
					{
						if ((c = Reader_getc(reader)) == EOF)
						{
							printf("Syntax error at line %d of input file \"%s\" - unterminated string.\n", Reader_getLineNumber(reader), Reader_getFilename(reader));
							exit(1);
						}
						else
						{
							value[len++] = c;
						}
					}
					else
					{
						value[len++] = c;
					}
				}
			}

			value[len] = '\0';
			create_lyric_event(get_track_by_number(midi_file, current_track), (long)(current_time), value);
		}
		else
		{
			char token[1024], *p, charvalue1, strvalue1[1024];
			int len = 0, value1, value2, value3, value4, value5;

			token[len++] = c;

			while ((c = Reader_getc(reader)) != EOF)
			{
 				if ((c == '#') || isspace(c))
				{
					Reader_ungetc(reader, c);
					break;
				}
				else
				{
					token[len++] = c;
				}
			}

			token[len] = '\0';

			if ((p = token), match_string(&p, "note=") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_note = value1 - 1;
			}
			else if ((p = token), match_string(&p, "note=x") && match_hex_number(&p, &value1) && (*p == '\0'))
			{
				current_note = value1;
			}
			else if ((p = token), match_string(&p, "note+") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_note += value1;
			}
			else if ((p = token), match_string(&p, "note-") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_note -= value1;
			}
			else if ((p = token), match_string(&p, "vel=") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_velocity = value1 - 1;
			}
			else if ((p = token), match_string(&p, "vel=x") && match_hex_number(&p, &value1) && (*p == '\0'))
			{
				current_velocity = value1;
			}
			else if ((p = token), match_string(&p, "vel=") && match_number(&p, &value1) && match_string(&p, "/") && match_number(&p, &value2) && (*p == '\0'))
			{
				current_velocity = (128 * value1 / value2) - 1;
			}
			else if ((p = token), match_string(&p, "vel+") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_velocity += value1;
			}
			else if ((p = token), match_string(&p, "vel+") && match_number(&p, &value1) && match_string(&p, "/") && match_number(&p, &value2) && (*p == '\0'))
			{
				current_velocity += (128 * value1 / value2);
			}
			else if ((p = token), match_string(&p, "vel-") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_velocity -= value1;
			}
			else if ((p = token), match_string(&p, "vel-") && match_number(&p, &value1) && match_string(&p, "/") && match_number(&p, &value2) && (*p == '\0'))
			{
				current_velocity -= (128 * value1 / value2);
			}
			else if ((p = token), match_string(&p, "vel*") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_velocity *= value1;
			}
			else if ((p = token), match_string(&p, "vel*") && match_number(&p, &value1) && match_string(&p, "/") && match_number(&p, &value2) && (*p == '\0'))
			{
				current_velocity *= value1 / value2;
			}
			else if ((p = token), match_string(&p, "vel/") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_velocity /= value1;
			}
			else if ((p = token), match_string(&p, "beat=") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_beats_per_measure = (float)(value1);
			}
			else if ((p = token), match_string(&p, "beat*") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_beats_per_measure *= (float)(value1);
			}
			else if ((p = token), match_string(&p, "beat/") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_beats_per_measure /= (float)(value1);
			}
			else if ((p = token), match_string(&p, "beat*") && match_number(&p, &value1) && match_string(&p, "/") && match_number(&p, &value2) && (*p == '\0'))
			{
				current_beats_per_measure *= (float)(value1) / (float)(value2);
			}
			else if ((p = token), match_string(&p, "time=") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_time = (float)(value1 - 1) * current_ticks_per_measure / current_beats_per_measure;
			}
			else if ((p = token), match_string(&p, "time=\"") && match_terminated_string(&p, '\"', strvalue1) && match_string(&p, "\"") && (*p == '\0'))
			{
				long marker_time = find_marker(midi_file, strvalue1);

				if (marker_time < 0)
				{
					printf("Error at line %d of input file \"%s\" - no such marker \"%s\".\n", Reader_getLineNumber(reader), Reader_getFilename(reader), strvalue1);
					exit(1);
				}

				current_time = (float)(marker_time);
			}
			else if ((p = token), match_string(&p, "time+") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_time += (float)(value1) * current_ticks_per_measure / current_beats_per_measure;
			}
			else if ((p = token), match_string(&p, "time-") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_time -= (float)(value1) * current_ticks_per_measure / current_beats_per_measure;
			}
			else if ((p = token), match_string(&p, "chan=") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_channel = value1 - 1;
			}
			else if ((p = token), match_string(&p, "chan=x") && match_hex_number(&p, &value1) && (*p == '\0'))
			{
				current_channel = value1;
			}
			else if ((p = token), match_string(&p, "track=") && match_number(&p, &value1) && (*p == '\0'))
			{
				current_track = value1;
			}
			else if ((p = token), match_string(&p, "prog=") && match_number(&p, &value1) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int program_number = value1 - 1;

				MidiFileTrack_createProgramChangeEvent(track, (long)(current_time), current_channel, program_number);
			}
			else if ((p = token), match_string(&p, "prog=x") && match_hex_number(&p, &value1) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int program_number = value1;

				MidiFileTrack_createProgramChangeEvent(track, (long)(current_time), current_channel, program_number);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")=") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int controller_value = value3 - 1;

				if (ramp_length > 0)
				{
					int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")=x") && match_hex_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int controller_value = value3;

				if (ramp_length > 0)
				{
					int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")=") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int controller_value = (128 * value3 / value4) - 1;

				if (ramp_length > 0)
				{
					int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")=") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int controller_value = (128 * value3 / value4) - 1;

				if (ramp_length > 0)
				{
					int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")+") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value + value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")+") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value + value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")+") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value + (128 * value3 / value4);

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);
	
					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")+") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value + (128 * value3 / value4);

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")-") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value - value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")-") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value - value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")-") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value - (128 * value3 / value4);

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")-") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value - (128 * value3 / value4);

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")*") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value * value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")*") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value * value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")*") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = (int)((float)(previous_controller_value) * (float)(value3) / (float)(value4));

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")*") && match_number(&p, &value3) && match_string(&p, "/") && match_number(&p, &value4) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = (int)((float)(previous_controller_value) * (float)(value3) / (float)(value4));

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(") && match_number(&p, &value2) && match_string(&p, ")/") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2 - 1;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value / value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "cc(x") && match_hex_number(&p, &value2) && match_string(&p, ")/") && match_number(&p, &value3) && (*p == '\0'))
			{
				MidiFileTrack_t track = get_track_by_number(midi_file, current_track);
				int ramp_length = value1;
				int controller_number = value2;
				int previous_controller_value = find_previous_controller_value(track, current_channel, controller_number, (long)(current_time));
				int controller_value = previous_controller_value / value3;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_controller_ramp(track, start_time, end_time, current_channel, controller_number, previous_controller_value, controller_value);
					current_time = end_time;
				}

				MidiFileTrack_createControlChangeEvent(track, (long)(current_time), current_channel, controller_number, controller_value);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "tempo=") && match_number(&p, &value2) && (*p == '\0'))
			{
				int ramp_length = value1;
				float tempo = (float)(value2);

				if (ramp_length > 0)
				{
					float previous_tempo = find_previous_tempo(midi_file, (long)(current_time));
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_tempo_ramp(midi_file, start_time, end_time, previous_tempo, tempo);
					current_time = end_time;
				}

				create_tempo_event(midi_file, (long)(current_time), tempo);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "tempo+") && match_number(&p, &value2) && (*p == '\0'))
			{
				int ramp_length = value1;
				float previous_tempo = find_previous_tempo(midi_file, (long)(current_time));
				float tempo = previous_tempo + value2;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_tempo_ramp(midi_file, start_time, end_time, previous_tempo, tempo);
					current_time = end_time;
				}

				create_tempo_event(midi_file, (long)(current_time), tempo);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "tempo-") && match_number(&p, &value2) && (*p == '\0'))
			{
				int ramp_length = value1;
				float previous_tempo = find_previous_tempo(midi_file, (long)(current_time));
				float tempo = previous_tempo - value2;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_tempo_ramp(midi_file, start_time, end_time, previous_tempo, tempo);
					current_time = end_time;
				}

				create_tempo_event(midi_file, (long)(current_time), tempo);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "tempo*") && match_number(&p, &value2) && (*p == '\0'))
			{
				int ramp_length = value1;
				float previous_tempo = find_previous_tempo(midi_file, (long)(current_time));
				float tempo = previous_tempo * value2;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_tempo_ramp(midi_file, start_time, end_time, previous_tempo, tempo);
					current_time = end_time;
				}

				create_tempo_event(midi_file, (long)(current_time), tempo);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "tempo*") && match_number(&p, &value2) && match_string(&p, "/") && match_number(&p, &value3) && (*p == '\0'))
			{
				int ramp_length = value1;
				float previous_tempo = find_previous_tempo(midi_file, (long)(current_time));
				float tempo = previous_tempo * (float)(value2) / (float)(value3);

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_tempo_ramp(midi_file, start_time, end_time, previous_tempo, tempo);
					current_time = end_time;
				}

				create_tempo_event(midi_file, (long)(current_time), tempo);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1), match_string(&p, "tempo/") && match_number(&p, &value2) && (*p == '\0'))
			{
				int ramp_length = value1;
				float previous_tempo = find_previous_tempo(midi_file, (long)(current_time));
				float tempo = previous_tempo / value2;

				if (ramp_length > 0)
				{
					float start_time = current_time;
					float end_time = current_time + ((float)(ramp_length) * current_ticks_per_measure / current_beats_per_measure);

					create_tempo_ramp(midi_file, start_time, end_time, previous_tempo, tempo);
					current_time = end_time;
				}

				create_tempo_event(midi_file, (long)(current_time), tempo);
			}
			else if ((p = token), match_string(&p, "artlen=") && match_number(&p, &value1) && match_string(&p, ",") && match_number(&p, &value2) && (*p == '\0'))
			{
				current_articulation_min_length = value1;
				current_articulation_gap_length = value2;
			}
			else if ((p = token), match_string(&p, "art=norm") && (*p == '\0'))
			{
				current_articulation_type = ARTICULATION_TYPE_NORMAL;
			}
			else if ((p = token), match_string(&p, "art=stac") && (*p == '\0'))
			{
				current_articulation_type = ARTICULATION_TYPE_STACCATO;
			}
			else if ((p = token), match_string(&p, "art=leg") && (*p == '\0'))
			{
				current_articulation_type = ARTICULATION_TYPE_LEGATO;
			}
			else if ((p = token), match_string(&p, "marker=\"") && match_terminated_string(&p, '\"', strvalue1) && match_string(&p, "\"") && (*p == '\0'))
			{
				create_marker_event(midi_file, (long)(current_time), strvalue1);
			}
			else if ((p = token), match_repeated_char(&p, '-', &value1) && (*p == '\0'))
			{
				current_time += (float)(value1) * current_ticks_per_measure / current_beats_per_measure;
			}
			else
			{
				int chord_note[1024], chord_duration, chord_velocity;
				ArticulationType_t chord_articulation_type;
				char *p2;
				len = 0;
				p = token;

				while (1)
				{
					if (p2 = p, match_repeated_char(&p2, '\'', &value1), match_repeated_char(&p2, ',', &value2), match_repeated_char(&p2, '+', &value3), match_repeated_char(&p2, '-', &value4), match_char_from_set(&p2, "1234567", &charvalue1))
					{
						int diatonic;

						switch (charvalue1)
						{
							case '1': diatonic = 0; break;
							case '2': diatonic = 2; break;
							case '3': diatonic = 4; break;
							case '4': diatonic = 5; break;
							case '5': diatonic = 7; break;
							case '6': diatonic = 9; break;
							case '7': diatonic = 11; break;
						}

						chord_note[len++] = current_note + (12 * value1) - (12 * value2) + value3 - value4 + diatonic;
						p = p2;
					}
					else if (p2 = p, match_string(&p2, "[") && match_number(&p2, &value1) && match_string(&p2, "]"))
					{
						chord_note[len++] = value1 - 1;
						p = p2;
					}
					else if (p2 = p, match_string(&p2, "[x") && match_hex_number(&p2, &value1) && match_string(&p2, "]"))
					{
						chord_note[len++] = value1;
						p = p2;
					}
					else if (p2 = p, match_repeated_char(&p2, '-', &value1), match_repeated_char(&p2, '<', &value2), match_repeated_char(&p2, '>', &value3), match_repeated_char(&p2, '!', &value4), match_repeated_char(&p2, '~', &value5), (*p2 == '\0'))
					{
						chord_duration = value1 + 1;
						chord_velocity = value2 - value3;

						if ((value4 == 0) && (value5 == 0))
						{
							chord_articulation_type = ARTICULATION_TYPE_NORMAL;
						}
						else if ((value4 == 1) && (value5 == 0))
						{
							chord_articulation_type = ARTICULATION_TYPE_STACCATO;
						}
						else if ((value4 == 0) && (value5 == 1))
						{
							chord_articulation_type = ARTICULATION_TYPE_LEGATO;
						}
						else
						{
							printf("Syntax error at line %d of input file \"%s\" - unrecognized token \"%s\".\n", Reader_getLineNumber(reader), Reader_getFilename(reader), token);
							exit(1);
						}

						break;
					}
					else
					{
						printf("Syntax error at line %d of input file \"%s\" - unrecognized token \"%s\".\n", Reader_getLineNumber(reader), Reader_getFilename(reader), token);
						exit(1);
					}
				}

				if (len == 0)
				{
					printf("Syntax error at line %d of input file \"%s\" - unrecognized token \"%s\".\n", Reader_getLineNumber(reader), Reader_getFilename(reader), token);
					exit(1);
				}
				else
				{
					int i;
					int articulated_chord_duration;

					if ((chord_articulation_type == ARTICULATION_TYPE_STACCATO) || ((chord_articulation_type == ARTICULATION_TYPE_NORMAL) && (current_articulation_type == ARTICULATION_TYPE_STACCATO)))
					{
						articulated_chord_duration = current_articulation_min_length;
					}
					else if ((chord_articulation_type == ARTICULATION_TYPE_LEGATO) || ((chord_articulation_type == ARTICULATION_TYPE_NORMAL) && (current_articulation_type == ARTICULATION_TYPE_LEGATO)))
					{
						articulated_chord_duration = (int)((float)(chord_duration) * current_ticks_per_measure / current_beats_per_measure);
					}
					else
					{
						articulated_chord_duration = (int)((float)(chord_duration) * current_ticks_per_measure / current_beats_per_measure) - current_articulation_gap_length;
						if (articulated_chord_duration < current_articulation_min_length) articulated_chord_duration = current_articulation_min_length;
					}

					for (i = 0; i < len; i++)
					{
						MidiFileTrack_createNoteOnEvent(get_track_by_number(midi_file, current_track), (long)(current_time), current_channel, chord_note[i], current_velocity + (chord_velocity * 16));
					}

					for (i = 0; i < len; i++)
					{
						MidiFileTrack_createNoteOffEvent(get_track_by_number(midi_file, current_track), (long)(current_time + articulated_chord_duration), current_channel, chord_note[i], 0);
					}

					current_time += (float)(chord_duration) * current_ticks_per_measure / current_beats_per_measure;
				}
			}
		}
	}

	Reader_free(reader);
}

void usage(char *program_name)
{
	printf("Usage: %s --in <input.mish> --out <output.mid>\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	char *input_filename = NULL;
	char *output_filename = NULL;
	MidiFile_t midi_file;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			input_filename = argv[i];
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (++i == argc) usage(argv[0]);
			output_filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if ((input_filename == NULL) || (output_filename == NULL)) usage(argv[0]);

	midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 120);
	process_mish(input_filename, midi_file);
	MidiFile_save(midi_file, output_filename);
	MidiFile_free(midi_file);
	return 0;
}


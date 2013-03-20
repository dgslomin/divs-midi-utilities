
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <sqlite3.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --sqlite-file <filename> --midi-file <filename>\n", program_name);
	exit(1);
}

static void emit_events(MidiFileEvent_t event, void *user_data)
{
	sqlite3 *sqlite_connection = (sqlite3 *)(user_data);

	switch (MidiFileEvent_getType(event))
	{
		case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
		{
			char sql_command[1024];
			sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('note_off', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileNoteOffEvent_getChannel(event), MidiFileNoteOffEvent_getNote(event), MidiFileNoteOffEvent_getVelocity(event));
			sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			break;
		}
		case MIDI_FILE_EVENT_TYPE_NOTE_ON:
		{
			MidiFileEvent_t note_end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

			if (note_end_event == NULL)
			{
				char sql_command[1024];
				sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('note_on', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
				sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			}
			else
			{
				char sql_command[1024];
				sprintf(sql_command, "INSERT INTO midi_events (type, tick, duration, track, channel, number, value) VALUES ('note_on', %d, %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileEvent_getTick(note_end_event) - MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
				sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			}
			break;
		}
		case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
		{
			char sql_command[1024];
			sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('key_pressure', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileKeyPressureEvent_getChannel(event), MidiFileKeyPressureEvent_getNote(event), MidiFileKeyPressureEvent_getAmount(event));
			sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			break;
		}
		case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
		{
			char sql_command[1024];
			sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('control_change', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileControlChangeEvent_getChannel(event), MidiFileControlChangeEvent_getNumber(event), MidiFileControlChangeEvent_getValue(event));
			sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			break;
		}
		case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
		{
			char sql_command[1024];
			sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, number) VALUES ('program_change', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileProgramChangeEvent_getChannel(event), MidiFileProgramChangeEvent_getNumber(event));
			sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			break;
		}
		case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
		{
			char sql_command[1024];
			sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, value) VALUES ('channel_pressure', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileChannelPressureEvent_getChannel(event), MidiFileChannelPressureEvent_getAmount(event));
			sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			break;
		}
		case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
		{
			char sql_command[1024];
			sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, channel, value) VALUES ('pitch_wheel', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFilePitchWheelEvent_getChannel(event), MidiFilePitchWheelEvent_getValue(event));
			sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			break;
		}
		case MIDI_FILE_EVENT_TYPE_SYSEX:
		{
			/* TODO */
			break;
		}
		case MIDI_FILE_EVENT_TYPE_META:
		{
			if (MidiFileEvent_isTempoEvent(event))
			{
				char sql_command[1024];
				sprintf(sql_command, "INSERT INTO midi_events (type, tick, track, float_value) VALUES ('tempo', %d, %d, %f);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileTempoEvent_getTempo(event));
				sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
			}
			else
			{
				/* TODO */
			}

			break;
		}
	}
}

int main(int argc, char **argv)
{
	int i;
	char *sqlite_filename = NULL;
	char *midi_filename = NULL;
	sqlite3 *sqlite_connection = NULL;
	MidiFile_t midi_file = NULL;
	MidiFileTrack_t track;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--sqlite-file") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sqlite_filename = argv[i];
		}
		else if (strcmp(argv[i], "--midi-file") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (sqlite_filename == NULL || midi_filename == NULL) usage(argv[0]);

	if (sqlite3_open(sqlite_filename, &sqlite_connection) != SQLITE_OK)
	{
		fprintf(stderr, "Error: Cannot open SQLite database \"%s\".\n", sqlite_filename);
		return 1;
	}

	{
		sqlite3_stmt *sqlite_prepared_statement = NULL;

		sqlite3_prepare_v2(sqlite_connection, "SELECT value, text FROM midi_events WHERE type = 'midi_file';", -1, &sqlite_prepared_statement, NULL);

		if (sqlite3_step(sqlite_prepared_statement) == SQLITE_ROW)
		{
			int resolution = sqlite3_column_int(sqlite_prepared_statement, 0);
			char *division_type_str = (char *)(sqlite3_column_text(sqlite_prepared_statement, 1));
			MidiFileDivisionType_t division_type = MIDI_FILE_DIVISION_TYPE_INVALID;

			if (strcmp(division_type_str, "PPQ") == 0)
			{
				division_type = MIDI_FILE_DIVISION_TYPE_PPQ;
			}
			else if (strcmp(division_type_str, "SMPTE24") == 0)
			{
				division_type = MIDI_FILE_DIVISION_TYPE_SMPTE24;
			}
			else if (strcmp(division_type_str, "SMPTE25") == 0)
			{
				division_type = MIDI_FILE_DIVISION_TYPE_SMPTE25;
			}
			else if (strcmp(division_type_str, "SMPTE30DROP") == 0)
			{
				division_type = MIDI_FILE_DIVISION_TYPE_SMPTE30DROP;
			}
			else if (strcmp(division_type_str, "SMPTE30") == 0)
			{
				division_type = MIDI_FILE_DIVISION_TYPE_SMPTE30;
			}

			midi_file = MidiFile_new(1, division_type, resolution);
		}

		sqlite3_finalize(sqlite_prepared_statement);
	}

	if (midi_file == NULL)
	{
		fprintf(stderr, "Error: No MIDI data found in database.\n");
		return 1;
	}

	{
		sqlite3_stmt *sqlite_prepared_statement = NULL;

		sqlite3_prepare_v2(sqlite_connection, "SELECT type, tick, duration, track, channel, number, value, float_value, text, data FROM midi_events;", -1, &sqlite_prepared_statement, NULL);

		while (sqlite3_step(sqlite_prepared_statement) == SQLITE_ROW)
		{
			char *type = (char *)(sqlite3_column_text(sqlite_prepared_statement, 0));

			if (strcmp(type, "note") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int duration = sqlite3_column_int(sqlite_prepared_statement, 2);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int note = sqlite3_column_int(sqlite_prepared_statement, 5);
				int velocity = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createNoteOnEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, note, velocity);
				MidiFileTrack_createNoteOffEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick + duration, channel, note, velocity);
			}
			else if (strcmp(type, "note_off") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int note = sqlite3_column_int(sqlite_prepared_statement, 5);
				int velocity = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createNoteOffEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, note, velocity);
			}
			else if (strcmp(type, "note_on") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int note = sqlite3_column_int(sqlite_prepared_statement, 5);
				int velocity = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createNoteOnEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, note, velocity);
			}
			else if (strcmp(type, "key_pressure") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int note = sqlite3_column_int(sqlite_prepared_statement, 5);
				int amount = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createKeyPressureEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, note, amount);
			}
			else if (strcmp(type, "control_change") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int number = sqlite3_column_int(sqlite_prepared_statement, 5);
				int value = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createControlChangeEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, number, value);
			}
			else if (strcmp(type, "program_change") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int number = sqlite3_column_int(sqlite_prepared_statement, 5);
				MidiFileTrack_createProgramChangeEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, number);
			}
			else if (strcmp(type, "channel_pressure") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int amount = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createChannelPressureEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, amount);
			}
			else if (strcmp(type, "pitch_wheel") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int channel = sqlite3_column_int(sqlite_prepared_statement, 4);
				int amount = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createPitchWheelEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, channel, amount);
			}
			else if (strcmp(type, "sysex") == 0)
			{
				/* TODO */
			}
			else if (strcmp(type, "tempo") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				float tempo = (float)(sqlite3_column_double(sqlite_prepared_statement, 7));
				MidiFileTrack_createTempoEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, tempo);
			}
			else if (strcmp(type, "time_signature") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				int numerator = sqlite3_column_int(sqlite_prepared_statement, 5);
				int denominator = sqlite3_column_int(sqlite_prepared_statement, 6);
				MidiFileTrack_createTimeSignatureEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, numerator, denominator);
			}
			else if (strcmp(type, "marker") == 0)
			{
				int tick = sqlite3_column_int(sqlite_prepared_statement, 1);
				int track_number = sqlite3_column_int(sqlite_prepared_statement, 3);
				char *text = (char *)(sqlite3_column_text(sqlite_prepared_statement, 8));
				MidiFileTrack_createMarkerEvent(MidiFile_getTrackByNumber(midi_file, track_number, 1), tick, text);
			}
			else if (strcmp(type, "meta") == 0)
			{
				/* TODO */
			}
		}

		sqlite3_finalize(sqlite_prepared_statement);
	}

	sqlite3_close(sqlite_connection);

	if (MidiFile_save(midi_file, midi_filename) < 0)
	{
		fprintf(stderr, "Error: Cannot write MIDI file \"%s\".\n", midi_filename);
		return 1;
	}

	MidiFile_free(midi_file);
	return 0;
}


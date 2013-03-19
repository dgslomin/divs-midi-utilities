
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <sqlite3.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --midi-file <filename> --sqlite-file <filename>\n", program_name);
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
	char *midi_filename = NULL;
	char *sqlite_filename = NULL;
	MidiFile_t midi_file;
	MidiFileTrack_t track;
	sqlite3 *sqlite_connection = NULL;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--midi-file") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_filename = argv[i];
		}
		else if (strcmp(argv[i], "--sqlite-file") == 0)
		{
			if (++i == argc) usage(argv[0]);
			sqlite_filename = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	if (midi_filename == NULL || sqlite_filename == NULL) usage(argv[0]);

	if ((midi_file = MidiFile_load(midi_filename)) == NULL)
	{
		fprintf(stderr, "Error: Cannot load MIDI file \"%s\".\n", midi_filename);
		return 1;
	}

	if (sqlite3_open(sqlite_filename, &sqlite_connection) != SQLITE_OK)
	{
		fprintf(stderr, "Error: Cannot open SQLite database \"%s\".\n", sqlite_filename);
		return 1;
	}

	sqlite3_exec(sqlite_connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	sqlite3_exec(sqlite_connection, "DROP TABLE IF EXISTS midi_events;", NULL, NULL, NULL);
	sqlite3_exec(sqlite_connection, "CREATE TABLE midi_events(type TEXT, tick INTEGER, duration INTEGER, track INTEGER, channel INTEGER, number INTEGER, value INTEGER, float_value FLOAT, text TEXT, data BLOB);", NULL, NULL, NULL);

	{
		char *division_type, sql_command[1024];

		switch (MidiFile_getDivisionType(midi_file))
		{
			case MIDI_FILE_DIVISION_TYPE_PPQ:
			{
				division_type = "PPQ";
				break;
			}
			case MIDI_FILE_DIVISION_TYPE_SMPTE24:
			{
				division_type = "SMPTE24";
				break;
			}
			case MIDI_FILE_DIVISION_TYPE_SMPTE25:
			{
				division_type = "SMPTE25";
				break;
			}
			case MIDI_FILE_DIVISION_TYPE_SMPTE30DROP:
			{
				division_type = "SMPTE30DROP";
				break;
			}
			case MIDI_FILE_DIVISION_TYPE_SMPTE30:
			{
				division_type = "SMPTE30";
				break;
			}
			default:
			{
				division_type = "";
				break;
			}
		}

		sprintf(sql_command, "INSERT INTO midi_events (type, text, value) VALUES ('midi_file', '%s', %d);", division_type, MidiFile_getResolution(midi_file));
		sqlite3_exec(sqlite_connection, sql_command, NULL, NULL, NULL);
	}

	for (track = MidiFile_getFirstTrack(midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		MidiFileTrack_visitEvents(track, emit_events, sqlite_connection);
	}

	sqlite3_exec(sqlite_connection, "COMMIT TRANSACTION;", NULL, NULL, NULL);
	sqlite3_close(sqlite_connection);
	MidiFile_free(midi_file);
	return 0;
}


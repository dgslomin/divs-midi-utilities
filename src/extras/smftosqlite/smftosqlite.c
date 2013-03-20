
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <sqlite3.h>

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --midi-file <filename> --sqlite-file <filename> [ --duration | --no-duration ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	char *midi_filename = NULL;
	char *sqlite_filename = NULL;
	int use_duration = 0;
	MidiFile_t midi_file;
	MidiFileEvent_t event;
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
		else if (strcmp(argv[i], "--duration") == 0)
		{
			use_duration = 1;
		}
		else if (strcmp(argv[i], "--no-duration") == 0)
		{
			use_duration = 0;
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
		char *division_type, sql_statement[1024];

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

		sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, text, value) VALUES ('midi_file', %Q, %d);", division_type, MidiFile_getResolution(midi_file));
		sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
	}

	for (event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		switch (MidiFileEvent_getType(event))
		{
			case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
			{
				MidiFileEvent_t note_start_event = MidiFileNoteEndEvent_getNoteStartEvent(event);

				if (!use_duration || (note_start_event == NULL))
				{
					char sql_statement[1024];
					sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('note_off', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileNoteOffEvent_getChannel(event), MidiFileNoteOffEvent_getNote(event), MidiFileNoteOffEvent_getVelocity(event));
					sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				}

				break;
			}
			case MIDI_FILE_EVENT_TYPE_NOTE_ON:
			{
				MidiFileEvent_t note_end_event = MidiFileNoteStartEvent_getNoteEndEvent(event);

				if (use_duration && (note_end_event != NULL))
				{
					char sql_statement[1024];
					sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, duration, track, channel, number, value) VALUES ('note', %d, %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileEvent_getTick(note_end_event) - MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
					sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				}
				else
				{
					char sql_statement[1024];
					sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('note_on', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
					sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				}

				break;
			}
			case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
			{
				char sql_statement[1024];
				sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('key_pressure', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileKeyPressureEvent_getChannel(event), MidiFileKeyPressureEvent_getNote(event), MidiFileKeyPressureEvent_getAmount(event));
				sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				break;
			}
			case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
			{
				char sql_statement[1024];
				sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, number, value) VALUES ('control_change', %d, %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileControlChangeEvent_getChannel(event), MidiFileControlChangeEvent_getNumber(event), MidiFileControlChangeEvent_getValue(event));
				sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				break;
			}
			case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
			{
				char sql_statement[1024];
				sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, number) VALUES ('program_change', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileProgramChangeEvent_getChannel(event), MidiFileProgramChangeEvent_getNumber(event));
				sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				break;
			}
			case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
			{
				char sql_statement[1024];
				sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, value) VALUES ('channel_pressure', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileChannelPressureEvent_getChannel(event), MidiFileChannelPressureEvent_getAmount(event));
				sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				break;
			}
			case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
			{
				char sql_statement[1024];
				sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, channel, value) VALUES ('pitch_wheel', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFilePitchWheelEvent_getChannel(event), MidiFilePitchWheelEvent_getValue(event));
				sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
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
					char sql_statement[1024];
					sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, float_value) VALUES ('tempo', %d, %d, %f);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileTempoEvent_getTempo(event));
					sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				}
				else if (MidiFileEvent_isTimeSignatureEvent(event))
				{
					char sql_statement[1024];
					sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, number, value) VALUES ('time_signature', %d, %d, %d, %d);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileTimeSignatureEvent_getNumerator(event), MidiFileTimeSignatureEvent_getDenominator(event));
					sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				}
				else if (MidiFileEvent_isMarkerEvent(event))
				{
					char sql_statement[1024];
					sqlite3_snprintf(1024, sql_statement, "INSERT INTO midi_events (type, tick, track, text) VALUES ('marker', %d, %d, %Q);", MidiFileEvent_getTick(event), MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)), MidiFileMarkerEvent_getText(event));
					sqlite3_exec(sqlite_connection, sql_statement, NULL, NULL, NULL);
				}
				else
				{
					/* TODO */
				}

				break;
			}
		}
	}

	sqlite3_exec(sqlite_connection, "COMMIT TRANSACTION;", NULL, NULL, NULL);
	sqlite3_close(sqlite_connection);
	MidiFile_free(midi_file);
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static void usage(char *program_name)
{
	printf("Usage: %s <filename>\n", program_name);
	exit(1);
}

static void display_events(MidiFileEvent_t event, void *user_data)
{
	switch (MidiFileEvent_getType(event))
	{
		case MIDI_FILE_EVENT_TYPE_NOTE_OFF:
		{
			printf("\t\t<NoteOffEvent Tick=\"%ld\" Channel=\"%d\" Note=\"%d\" Velocity=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFileNoteOffEvent_getChannel(event), MidiFileNoteOffEvent_getNote(event), MidiFileNoteOffEvent_getVelocity(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_NOTE_ON:
		{
			printf("\t\t<NoteOnEvent Tick=\"%ld\" Channel=\"%d\" Note=\"%d\" Velocity=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFileNoteOnEvent_getChannel(event), MidiFileNoteOnEvent_getNote(event), MidiFileNoteOnEvent_getVelocity(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_KEY_PRESSURE:
		{
			printf("\t\t<KeyPressureEvent Tick=\"%ld\" Channel=\"%d\" Note=\"%d\" Amount=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFileKeyPressureEvent_getChannel(event), MidiFileKeyPressureEvent_getNote(event), MidiFileKeyPressureEvent_getAmount(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE:
		{
			printf("\t\t<ControlChangeEvent Tick=\"%ld\" Channel=\"%d\" Number=\"%d\" Value=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFileControlChangeEvent_getChannel(event), MidiFileControlChangeEvent_getNumber(event), MidiFileControlChangeEvent_getValue(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_PROGRAM_CHANGE:
		{
			printf("\t\t<ProgramChangeEvent Tick=\"%ld\" Channel=\"%d\" Number=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFileProgramChangeEvent_getChannel(event), MidiFileProgramChangeEvent_getNumber(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_CHANNEL_PRESSURE:
		{
			printf("\t\t<ChannelPressureEvent Tick=\"%ld\" Channel=\"%d\" Amount=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFileChannelPressureEvent_getChannel(event), MidiFileChannelPressureEvent_getAmount(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_PITCH_WHEEL:
		{
			printf("\t\t<PitchWheelEvent Tick=\"%ld\" Channel=\"%d\" Value=\"%d\"/>\n", MidiFileEvent_getTick(event), MidiFilePitchWheelEvent_getChannel(event), MidiFilePitchWheelEvent_getValue(event));
			break;
		}
		case MIDI_FILE_EVENT_TYPE_SYSEX:
		{
			int i, data_length = MidiFileSysexEvent_getDataLength(event);
			unsigned char *data = MidiFileSysexEvent_getData(event);
			printf("\t\t<SysexEvent Tick=\"%ld\" DataLength=\"%d\" Data=\"", MidiFileEvent_getTick(event), MidiFileSysexEvent_getDataLength(event));

			for (i = 0; i < data_length; i++)
			{
				if (((data[i] >= 'a') && (data[i] <= 'z')) || ((data[i] >= 'A') && (data[i] <= 'Z')) || ((data[i] >= '0') && (data[i] <= '9')) || (data[i] == '-') || (data[i] == '_') || (data[i] == '.') || (data[i] == '~'))
				{
					putchar(data[i]);
				}
				else
				{
					printf("%%%02X", data[i]);
				}
			}

			printf("\"/>\n");
			break;
		}
		case MIDI_FILE_EVENT_TYPE_META:
		{
			int i, data_length = MidiFileMetaEvent_getDataLength(event);
			unsigned char *data = MidiFileMetaEvent_getData(event);
			printf("\t\t<MetaEvent Tick=\"%ld\" Number=\"%d\" DataLength=\"%d\" Data=\"", MidiFileEvent_getTick(event), MidiFileMetaEvent_getNumber(event), MidiFileMetaEvent_getDataLength(event));

			for (i = 0; i < data_length; i++)
			{
				if (((data[i] >= 'a') && (data[i] <= 'z')) || ((data[i] >= 'A') && (data[i] <= 'Z')) || ((data[i] >= '0') && (data[i] <= '9')) || (data[i] == '-') || (data[i] == '_') || (data[i] == '.') || (data[i] == '~'))
				{
					putchar(data[i]);
				}
				else
				{
					printf("%%%02X", data[i]);
				}
			}

			printf("\"/>\n");
			break;
		}
		default:
		{
			break;
		}
	}
}

int main(int argc, char **argv)
{
	char *filename;
	MidiFile_t midi_file;
	MidiFileTrack_t track;

	if (argc != 2) usage(argv[0]);
	filename = argv[1];

	if ((midi_file = MidiFile_load(filename)) == NULL)
	{
		printf("Error: Cannot load MIDI file \"%s\".\n", filename);
		return 1;
	}

	printf("<?xml version=\"1.0\"?>\n");
	printf("<MidiFile DivisionType=\"");

	switch (MidiFile_getDivisionType(midi_file))
	{
		case MIDI_FILE_DIVISION_TYPE_PPQ:
		{
			printf("PPQ");
			break;
		}
		case MIDI_FILE_DIVISION_TYPE_SMPTE24:
		{
			printf("SMPTE24");
			break;
		}
		case MIDI_FILE_DIVISION_TYPE_SMPTE25:
		{
			printf("SMPTE25");
			break;
		}
		case MIDI_FILE_DIVISION_TYPE_SMPTE30DROP:
		{
			printf("SMPTE30DROP");
			break;
		}
		case MIDI_FILE_DIVISION_TYPE_SMPTE30:
		{
			printf("SMPTE30");
			break;
		}
		default:
		{
			break;
		}
	}

	printf("\" Resolution=\"%d\">\n", MidiFile_getResolution(midi_file));

	for (track = MidiFile_getFirstTrack(midi_file); track != NULL; track = MidiFileTrack_getNextTrack(track))
	{
		printf("\t<Track EndTick=\"%ld\">\n", MidiFileTrack_getEndTick(track));
		MidiFileTrack_visitEvents(track, display_events, NULL);
		printf("\t</Track>\n");
	}

	printf("</MidiFile>\n");

	MidiFile_free(midi_file);
	return 0;
}


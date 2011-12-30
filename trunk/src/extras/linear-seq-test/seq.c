
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>

static int _compare_midi_file_events(const void *a, const void *b)
{
	MidiFileEvent_t aa = *((MidiFileEvent_t *)(a));
	MidiFileEvent_t bb = *((MidiFileEvent_t *)(b));

	if (MidiFileEvent_isNoteStartEvent(aa) && MidiFileEvent_isNoteStartEvent(bb))
	{
		return MidiFileNoteStartEvent_getNote(bb) - MidiFileNoteStartEvent_getNote(aa);
	}
	else
	{
		return 0;
	}
}

int main(int argc, char **argv)
{
	MidiFile_t midi_file;
	float beat_size = 0.125;
	float beat = 0.0;
	MidiFileEvent_t event;
	int number_of_events_in_beat = 0;
	MidiFileEvent_t *events_in_beat = NULL;
	int event_number;

	midi_file = MidiFile_load(argv[1]);

	printf("<html>\n");
	printf("<head>\n");
	printf("\n");
	printf("<style type=\"text/css\">\n");
	printf("body, table { font-size: 9pt; font-family: tahoma }\n");
	printf("table { border-collapse: collapse; margin-left: auto; margin-right: auto; border: solid 1px #cccccc }\n");
	printf("tr.beat { border-top: solid 1px #cccccc }\n");
	printf("td { padding: 0 2ex; text-align: center }\n");
	printf("</style>\n");
	printf("\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("\n");
	printf("<table>\n");
	printf("\n");

	for (event = MidiFile_getFirstEvent(midi_file); event != NULL; beat += beat_size)
	{
		while (event != NULL)
		{
			float actual_beat = MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(event));

			if ((actual_beat >= beat - (beat_size / 2)) && (actual_beat < beat + (beat_size / 2)))
			{
				if (MidiFileEvent_isNoteStartEvent(event) && (MidiFileTrack_getNumber(MidiFileEvent_getTrack(event)) == 1))
				{
					events_in_beat = realloc(events_in_beat, sizeof (MidiFileEvent_t) * (number_of_events_in_beat + 1));
					events_in_beat[number_of_events_in_beat] = event;
					number_of_events_in_beat++;
				}

				event = MidiFileEvent_getNextEventInFile(event);
			}
			else
			{
				break;
			}
		}

		if (number_of_events_in_beat == 0)
		{
			printf("<tr class=\"beat\">\n");
			printf("<td>%.3f</td>\n", beat);
			printf("<td></td>\n");
			printf("<td></td>\n");
			printf("<td></td>\n");
			printf("<td></td>\n");
			printf("<td></td>\n");
			printf("</tr>\n");
			printf("\n");
		}
		else
		{
			qsort(events_in_beat, number_of_events_in_beat, sizeof (MidiFileEvent_t), _compare_midi_file_events);

			for (event_number = 0; event_number < number_of_events_in_beat; event_number++)
			{
				float actual_beat = MidiFile_getBeatFromTick(midi_file, MidiFileEvent_getTick(events_in_beat[event_number]));
				int track_number = MidiFileTrack_getNumber(MidiFileEvent_getTrack(events_in_beat[event_number]));
				int channel = MidiFileNoteStartEvent_getChannel(events_in_beat[event_number]) + 1;
				int note_number = MidiFileNoteStartEvent_getNote(events_in_beat[event_number]);
				int velocity = MidiFileNoteStartEvent_getVelocity(events_in_beat[event_number]);

				if (event_number == 0)
				{
					printf("<tr class=\"beat\">\n");
				}
				else
				{
					printf("<tr>\n");
				}

				printf("<td>%.3f</td>\n", actual_beat);
				printf("<td>NoteStart</td>\n");
				printf("<td>%d</td>\n", track_number);
				printf("<td>%d</td>\n", channel);
				printf("<td>%s%d</td>\n", "C\0\0C#\0D\0\0D#\0E\0\0F\0\0F#\0G\0\0G#\0A\0\0A#\0B\0\0" + ((note_number % 12) * 3), note_number / 12);
				printf("<td>%d</td>\n", velocity);
				printf("</tr>\n");
				printf("\n");
			}

			number_of_events_in_beat = 0;
		}
	}

	printf("</table>\n");
	printf("\n");
	printf("</body>\n");
	printf("</html>\n");
	printf("\n");

	if (events_in_beat != NULL) free(events_in_beat);
	MidiFile_free(midi_file);
	return 0;
}


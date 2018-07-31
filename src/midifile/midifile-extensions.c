
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <midifile-extensions.h>

MidiFileEvent_t MidiFile_getCaretEvent(MidiFile_t midi_file, int create)
{
	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isCaretEvent(event)) return event;
	}

	if (create)
	{
		return MidiFileTrack_createCaretEvent(MidiFile_getTrackByNumber(midi_file, 0, 1), 0);
	}
	else
	{
		return NULL;
	}
}

MidiFileEvent_t MidiFileTrack_createCaretEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_CARET_LABEL);
}

int MidiFileEvent_isCaretEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_CARET_LABEL) == 0));
}

int MidiFileCaretEvent_setTargetEvent(MidiFileEvent_t event, MidiFileEvent_t target_event)
{
	if ((! MidiFileEvent_isCaretEvent(event)) || (target_event == NULL)) return -1;
	MidiFileEvent_setNextEvent(event, target_event);
	return 0;
}

MidiFileEvent_t MidiFileCaretEvent_getTargetEvent(MidiFileEvent_t event)
{
	MidiFileEvent_t next_event;
	if (! MidiFileEvent_isCaretEvent(event)) return NULL;
	next_event = MidiFileEvent_getNextEventInTrack(event);
	if ((next_event == NULL) || (MidiFileEvent_getTick(next_event) != MidiFileEvent_getTick(event))) return NULL;
	return next_event;
}

int MidiFile_hasSelection(MidiFile_t midi_file)
{
	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isSelectionStartEvent(event)) return 1;
	}

	return 0;
}

int MidiFile_clearSelection(MidiFile_t midi_file)
{
	MidiFileEvent_t next_event;

	if (midi_file == NULL) return -1;

	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = next_event)
	{
		next_event = MidiFileEvent_getNextEventInFile(event);
		if (MidiFileEvent_isSelectionStartEvent(event) || MidiFileEvent_isSelectionEndEvent(event)) MidiFileEvent_delete(event);
	}

	return 0;
}

MidiFileEvent_t MidiFileTrack_createSelectionStartEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_SELECTION_START_LABEL);
}

MidiFileEvent_t MidiFileTrack_createSelectionEndEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_SELECTION_END_LABEL);
}

int MidiFileEvent_isSelected(MidiFileEvent_t event)
{
	while (event != NULL)
	{
		if (MidiFileEvent_isSelectionStartEvent(event))
		{
			return 1;
		}
		else if (MidiFileEvent_isSelectionEndEvent(event))
		{
			return 0;
		}

		event = MidiFileEvent_getPreviousEventInTrack(event);
	}

	return 0;
}

int MidiFileEvent_setSelected(MidiFileEvent_t event, int selected)
{
	if (event == NULL) return -1;

	if (MidiFileEvent_isSelected(event))
	{
		if (!selected)
		{
			MidiFileEvent_setNextEvent(MidiFileTrack_createSelectionEndEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event)), event);
			MidiFileEvent_setPreviousEvent(MidiFileTrack_createSelectionStartEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event)), event);
		}
	}
	else
	{
		if (selected)
		{
			MidiFileEvent_setNextEvent(MidiFileTrack_createSelectionStartEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event)), event);
			MidiFileEvent_setPreviousEvent(MidiFileTrack_createSelectionEndEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event)), event);
		}
	}

	return 0;
}

int MidiFileEvent_isSelectionStartEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_SELECTION_START_LABEL) == 0));
}

int MidiFileEvent_isSelectionEndEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_SELECTION_END_LABEL) == 0));
}

MidiFileEvent_t MidiFileSelectionStartEvent_getSelectionEndEvent(MidiFileEvent_t event)
{
	if (! MidiFileEvent_isSelectionStartEvent(event)) return NULL;

	for (MidiFileEvent_t subsequent_event = MidiFileEvent_getNextEvent(event); subsequent_event != NULL; subsequent_event = MidiFileEvent_getNextEventInTrack(subsequent_event))
	{
		if (MidiFileEvent_isSelectionEndEvent(subsequent_event)) return subsequent_event;
	}

	return NULL;
}

MidiFileEvent_t MidiFileSelectionEndEvent_getSelectionStartEvent(MidiFileEvent_t event)
{
	if (! MidiFileEvent_isSelectionEndEvent(event)) return NULL;

	for (MidiFileEvent_t preceding_event = MidiFileEvent_getPreviousEvent(event); preceding_event != NULL; preceding_event = MidiFileEvent_getPreviousEventInTrack(preceding_event))
	{
		if (MidiFileEvent_isSelectionStartEvent(preceding_event)) return preceding_event;
	}

	return NULL;
}


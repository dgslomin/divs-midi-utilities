
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <midifile-extensions.h>

MidiFileEvent_t MidiFile_getCaretTarget(MidiFile_t midi_file)
{
	for (MidiFileEvent_t caret_event = MidiFile_getFirstEvent(midi_file); caret_event != NULL; caret_event = MidiFileEvent_getNextEventInFile(caret_event))
	{
		if (MidiFileEvent_isCaretEvent(caret_event)) return MidiFile_getNextEventInTrack(caret_event);
	}

	return NULL;
}

int MidiFile_clearCaretTarget(MidiFile_t midi_file)
{
	MidiFileEvent_t next_event;

	if (midi_file == NULL) return -1;

	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = next_event)
	{
		next_event = MidiFileEvent_getNextEventInFile(event);
		if (MidiFileEvent_isCaretEvent(event)) MidiFileEvent_delete(event);
	}

	return 0;
}

int MidiFileEvent_isCaretTarget(MidiFileEvent_t event)
{
	return MidiFileEvent_isCaretEvent(MidiFileEvent_getPreviousEventInTrack(event));
}

int MidiFile_setCaretTarget(MidiFileEvent_t event)
{
	if (event == NULL) return -1;
	MidiFile_clearCaretTarget(MidiFileTrack_getMidiFile(MidiFileEvent_getTrack(event)));
	MidiFileEvent_setNextEvent(MidiFileTrack_createTextEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event), MIDI_FILE_CARET_LABEL), event);
	return 0;
}

int MidiFileEvent_isCaretEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_CARET_LABEL) == 0));
}

int MidiFile_hasSelection(MidiFile_t midi_file)
{
	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isSelectionEvent(event)) return 1;
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
		if (MidiFileEvent_isSelectionEvent(event)) MidiFileEvent_delete(event);
	}

	return 0;
}

int MidiFileEvent_isSelectionEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_SELECTION_LABEL) == 0));
}

int MidiFileEvent_isSelected(MidiFileEvent_t event)
{
	return MidiFileEvent_isSelectionEvent(MidiFileEvent_getPreviousEventInTrack(event));
}

int MidiFileEvent_setSelected(MidiFileEvent_t event, int selected)
{
	if (event == NULL) return -1;

	if (MidiFileEvent_isSelected(event))
	{
		if (!selected)
		{
			MidiFileEvent_delete(MidiFileEvent_getPreviousEventInTrack(event));
		}
	}
	else
	{
		if (selected)
		{
			MidiFileEvent_setNextEvent(MidiFileTrack_createTextEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event), MIDI_FILE_SELECTION_LABEL), event);
		}
	}

	return 0;
}

MidiFileEvent_t MidiFileTrack_createEmptyTargetEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_EMPTY_TARGET_LABEL)
}

int MidiFileEvent_isEmptyTargetEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_EMPTY_TARGET_LABEL) == 0));
}


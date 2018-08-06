
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <midifile.h>
#include <midifile-extensions.h>

MidiFileEvent_t MidiFileTrack_createCaretAnnotationEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_CARET_ANNOTATION_LABEL);
}

int MidiFileEvent_isCaretAnnotationEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_CARET_ANNOTATION_LABEL) == 0));
}

MidiFileEvent_t MidiFile_getCaret(MidiFile_t midi_file)
{
	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isCaret(event)) return event;
	}

	return NULL;
}

int MidiFile_clearCaret(MidiFile_t midi_file)
{
	if (midi_file == NULL) return -1;

	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isCaret(event))
		{
			MidiFileEvent_setCaret(event, 0);
			break;
		}
	}

	return 0;
}

int MidiFileEvent_isCaret(MidiFileEvent_t event)
{
	for (MidiFileEvent_t preceding_event = MidiFileEvent_getPreviousEventInTrack(event); (preceding_event != NULL) && (MidiFileEvent_getTick(preceding_event) == MidiFileEvent_getTick(event)); preceding_event = MidiFileEvent_getPreviousEventInTrack(preceding_event))
	{
		if (MidiFileEvent_isCaretAnnotationEvent(preceding_event))
		{
			return 1;
		}
		else if (!MidiFileEvent_isSelectionAnnotationEvent(preceding_event))
		{
			break;
		}
	}

	return 0;
}

int MidiFileEvent_setCaret(MidiFileEvent_t event, int caret)
{
	if (event == NULL) return -1;

	if (caret)
	{
		if (!MidiFileEvent_isCaret(event))
		{
			MidiFileEvent_setNextEvent(MidiFileTrack_createCaretAnnotationEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event)), event);
		}
	}
	else
	{
		for (MidiFileEvent_t preceding_event = MidiFileEvent_getPreviousEventInTrack(event); (preceding_event != NULL) && (MidiFileEvent_getTick(preceding_event) == MidiFileEvent_getTick(event)); preceding_event = MidiFileEvent_getPreviousEventInTrack(preceding_event))
		{
			if (MidiFileEvent_isCaretAnnotationEvent(preceding_event))
			{
				MidiFileEvent_delete(preceding_event);
				break;
			}
			else if (!MidiFileEvent_isSelectionAnnotationEvent(preceding_event))
			{
				break;
			}
		}
	}

	return 0;
}

MidiFileEvent_t MidiFileTrack_createSelectionAnnotationEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_SELECTION_ANNOTATION_LABEL);
}

int MidiFileEvent_isSelectionAnnotationEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_SELECTION_ANNOTATION_LABEL) == 0));
}

MidiFileEvent_t MidiFile_getFirstSelection(MidiFile_t midi_file)
{
	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isSelected(event)) return event;
	}

	return NULL;
}

int MidiFile_clearSelections(MidiFile_t midi_file)
{
	if (midi_file == NULL) return -1;

	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isSelected(event)) MidiFileEvent_setSelected(event, 0);
	}

	return 0;
}

MidiFileEvent_t MidiFileTrack_getFirstSelection(MidiFileTrack_t track)
{
	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(track); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isSelected(event)) return event;
	}

	return NULL;
}

int MidiFileTrack_clearSelections(MidiFileTrack_t track)
{
	if (track == NULL) return -1;

	for (MidiFileEvent_t event = MidiFileTrack_getFirstEvent(track); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isSelected(event)) MidiFileEvent_setSelected(event, 0);
	}

	return 0;
}

MidiFileEvent_t MidiFileEvent_getNextSelectionInTrack(MidiFileEvent_t event)
{
	for (event = MidiFileEvent_getNextEventInTrack(event); event != NULL; event = MidiFileEvent_getNextEventInTrack(event))
	{
		if (MidiFileEvent_isSelected(event)) return event;
	}

	return NULL;
}

MidiFileEvent_t MidiFileEvent_getNextSelectionInFile(MidiFileEvent_t event)
{
	for (event = MidiFileEvent_getNextEventInFile(event); event != NULL; event = MidiFileEvent_getNextEventInFile(event))
	{
		if (MidiFileEvent_isSelected(event)) return event;
	}

	return NULL;
}

int MidiFileEvent_isSelected(MidiFileEvent_t event)
{
	for (MidiFileEvent_t preceding_event = MidiFileEvent_getPreviousEventInTrack(event); (preceding_event != NULL) && (MidiFileEvent_getTick(preceding_event) == MidiFileEvent_getTick(event)); preceding_event = MidiFileEvent_getPreviousEventInTrack(preceding_event))
	{
		if (MidiFileEvent_isSelectionAnnotationEvent(preceding_event))
		{
			return 1;
		}
		else if (!MidiFileEvent_isCaretAnnotationEvent(preceding_event))
		{
			break;
		}
	}

	return 0;
}

int MidiFileEvent_setSelected(MidiFileEvent_t event, int selected)
{
	if (event == NULL) return -1;

	if (selected)
	{
		if (!MidiFileEvent_isSelected(event))
		{
			MidiFileEvent_setNextEvent(MidiFileTrack_createSelectionAnnotationEvent(MidiFileEvent_getTrack(event), MidiFileEvent_getTick(event)), event);
		}
	}
	else
	{
		for (MidiFileEvent_t preceding_event = MidiFileEvent_getPreviousEventInTrack(event); (preceding_event != NULL) && (MidiFileEvent_getTick(preceding_event) == MidiFileEvent_getTick(event)); preceding_event = MidiFileEvent_getPreviousEventInTrack(preceding_event))
		{
			if (MidiFileEvent_isSelectionAnnotationEvent(preceding_event))
			{
				MidiFileEvent_delete(preceding_event);
				break;
			}
			else if (!MidiFileEvent_isCaretAnnotationEvent(preceding_event))
			{
				break;
			}
		}
	}

	return 0;
}

MidiFileEvent_t MidiFileTrack_createEmptyAnnotationTargetEvent(MidiFileTrack_t track, long tick)
{
	return MidiFileTrack_createTextEvent(track, tick, MIDI_FILE_EMPTY_ANNOTATION_TARGET_LABEL);
}

int MidiFileEvent_isEmptyAnnotationTargetEvent(MidiFileEvent_t event)
{
	return (MidiFileEvent_isTextEvent(event) && (strcmp(MidiFileTextEvent_getText(event), MIDI_FILE_EMPTY_ANNOTATION_TARGET_LABEL) == 0));
}

int MidiFile_deleteOrphanEmptyAnnotationTargetEvents(MidiFile_t midi_file)
{
	MidiFileEvent_t next_event;

	if (midi_file == NULL) return -1;

	for (MidiFileEvent_t event = MidiFile_getFirstEvent(midi_file); event != NULL; event = next_event)
	{
		next_event = MidiFileEvent_getNextEventInFile(event);
		if (MidiFileEmptyAnnotationTargetEvent_isOrphan(event)) MidiFileEvent_delete(event);
	}

	return 0;
}

int MidiFileEmptyAnnotationTargetEvent_isOrphan(MidiFileEvent_t event)
{
	return (MidiFileEvent_isEmptyAnnotationTargetEvent(event) && !MidiFileEvent_isCaret(event) && !MidiFileEvent_isSelected(event));
}

int MidiFileEvent_deleteWithAnnotations(MidiFileEvent_t event)
{
	if (event == NULL) return -1;
	MidiFileEvent_setCaret(event, 0);
	MidiFileEvent_setSelected(event, 0);
	MidiFileEvent_delete(event);
	return 0;
}

int MidiFileEvent_setTrackWithAnnotations(MidiFileEvent_t event, MidiFileTrack_t track)
{
	int caret;
	int selected;

	if ((event == NULL) || (track == NULL)) return -1;
	caret = MidiFileEvent_isCaret(event);
	selected = MidiFileEvent_isSelected(event);
	if (caret) MidiFileEvent_setCaret(event, 0);
	if (selected) MidiFileEvent_setSelected(event, 0);
	MidiFileEvent_setTrack(event, track);
	if (caret) MidiFileEvent_setCaret(event, 1);
	if (selected) MidiFileEvent_setSelected(event, 1);
	return 0;
}

int MidiFileEvent_setTickWithAnnotations(MidiFileEvent_t event, long tick)
{
	int caret;
	int selected;

	if ((event == NULL) || (tick < 0)) return -1;
	caret = MidiFileEvent_isCaret(event);
	selected = MidiFileEvent_isSelected(event);
	if (caret) MidiFileEvent_setCaret(event, 0);
	if (selected) MidiFileEvent_setSelected(event, 0);
	MidiFileEvent_setTick(event, tick);
	if (caret) MidiFileEvent_setCaret(event, 1);
	if (selected) MidiFileEvent_setSelected(event, 1);
	return 0;
}


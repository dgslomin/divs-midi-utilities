#ifndef MIDI_FILE_EXTENSIONS_INCLUDED
#define MIDI_FILE_EXTENSIONS_INCLUDED

/*
 * Div's Standard MIDI File API
 * Copyright 2003-2018 by David G. Slomin
 * Provided under the terms of the BSD license
 *
 * These are extensions that are not part of the MIDI file standard, but are
 * built on top of the primitives provided there (text events with particular
 * labels in them).  Files that use these events will be compatible with other
 * MIDI software, but the extension events will not have any special meaning
 * there.
 *
 * The caret is the editing "cursor", where subsequent inserts should occur.
 * If there is no selection and there is an event immediately following the
 * caret at the same track and tick, then that following event should be the
 * target of editing operations.  There should be only one caret; extras after
 * the first are ignored.
 * 
 * Selections are the editing "highlight", for marking multiple events or time
 * ranges as the target of editing operations.  There can be multiple
 * discontiguous selections at once.
 */
#ifdef __cplusplus
extern "C"
{
#endif

#define MIDI_FILE_CARET_LABEL "!caret"

MidiFileEvent_t MidiFile_getCaretEvent(MidiFile_t midi_file, int create);
MidiFileEvent_t MidiFileTrack_createCaretEvent(MidiFileTrack_t track, long tick);
int MidiFileEvent_isCaretEvent(MidiFileEvent_t event);
int MidiFileCaretEvent_setTargetEvent(MidiFileEvent_t event, MidiFileEvent_t target_event);
MidiFileEvent_t MidiFileCaretEvent_getTargetEvent(MidiFileEvent_t event);

#define MIDI_FILE_SELECTION_START_LABEL "!selection-start"
#define MIDI_FILE_SELECTION_END_LABEL "!selection-end"

int MidiFile_hasSelection(MidiFile_t midi_file);
int MidiFile_clearSelection(MidiFile_t midi_file);
MidiFileEvent_t MidiFileTrack_createSelectionStartEvent(MidiFileTrack_t track, long tick);
MidiFileEvent_t MidiFileTrack_createSelectionEndEvent(MidiFileTrack_t track, long tick);
int MidiFileEvent_isSelected(MidiFileEvent_t event);
int MidiFileEvent_setSelected(MidiFileEvent_t event, int selected);
int MidiFileEvent_isSelectionStartEvent(MidiFileEvent_t event);
int MidiFileEvent_isSelectionEndEvent(MidiFileEvent_t event);
MidiFileEvent_t MidiFileSelectionStartEvent_getSelectionEndEvent(MidiFileEvent_t event);
MidiFileEvent_t MidiFileSelectionEndEvent_getSelectionStartEvent(MidiFileEvent_t event);

#ifdef __cplusplus
}
#endif

#endif

#ifndef MIDI_FILE_EXTENSIONS_INCLUDED
#define MIDI_FILE_EXTENSIONS_INCLUDED

/*
 * Div's Standard MIDI File API
 * Copyright 2003-2018 by David G. Slomin
 * Provided under the terms of the BSD license
 *
 * These are extensions that are not part of the MIDI file standard, but are
 * built on top of the primitives provided there.  Files that use these events
 * will be compatible with other MIDI software, but the extension events will
 * not have any special meaning there.
 *
 * The caret is the editing "cursor", which marks where inserts should occur.
 * It also marks the target of editing operations when there is no selection.
 * It is represented by a specially labelled text event located immediately
 * prior to the target event.  There can only be one caret at a time.
 *
 * Selections are the editing "highlight", for marking events as the target of
 * editing operations.  It is represented by a specially labelled text event
 * located immediately prior to each target event.  There can be many events
 * selected at the same time, and they do not have to be contiguous.
 *
 * An empty target event is useful if you want the cursor or selection to
 * target a tick where no real event occurs.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define MIDI_FILE_CARET_LABEL "!caret"

MidiFileEvent_t MidiFile_getCaretTarget(MidiFile_t midi_file);
int MidiFile_clearCaretTarget(MidiFile_t midi_file);
int MidiFileEvent_isCaretTarget(MidiFileEvent_t event);
int MidiFileEvent_setCaretTarget(MidiFileEvent_t event);
int MidiFileEvent_isCaretEvent(MidiFileEvent_t event);

#define MIDI_FILE_SELECTION_LABEL "!selection"

int MidiFile_hasSelection(MidiFile_t midi_file);
int MidiFile_clearSelection(MidiFile_t midi_file);
int MidiFileEvent_isSelected(MidiFileEvent_t event);
int MidiFileEvent_setSelected(MidiFileEvent_t event, int selected);
int MidiFileEvent_isSelectionEvent(MidiFileEvent_t event);

#define MIDIFILE_EMPTY_TARGET_LABEL "!empty-target"

MidiFileEvent_t MidiFileTrack_createEmptyTargetEvent(MidiFileTrack_t track, long tick);
int MidiFileEvent_isEmptyTargetEvent(MidiFileEvent_t event);

#ifdef __cplusplus
}
#endif

#endif

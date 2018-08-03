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
 * It is represented by an annotation - a specially labelled text event located
 * immediately prior to the target event.  There can only be one caret at a
 * time.
 *
 * Selections are the editing "highlight", for marking events as the target of
 * editing operations.  It is represented by an annotation - a specially
 * labelled text event located immediately prior to each target event.  There
 * can be many events selected at the same time, and they do not have to be
 * contiguous.
 *
 * An empty annotation target event is useful if you want the cursor or
 * selection to target a tick where no real event occurs.  Don't forget to
 * clean up the empty annotation targets that have become orphaned when their
 * annotations have been removed.
 *
 * When you delete or move an event in the file, it makes sense to delete or
 * move the annotations along with it.  Helper functions are provided for this.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define MIDI_FILE_CARET_ANNOTATION_LABEL "!caret"

MidiFileEvent_t MidiFileTrack_createCaretAnnotationEvent(MidiFileTrack_t track, long tick);
int MidiFileEvent_isCaretAnnotationEvent(MidiFileEvent_t event);

MidiFileEvent_t MidiFile_getCaret(MidiFile_t midi_file);
int MidiFile_clearCaret(MidiFile_t midi_file);
int MidiFileEvent_isCaret(MidiFileEvent_t event);
int MidiFileEvent_setCaret(MidiFileEvent_t event, int caret);



#define MIDI_FILE_SELECTION_ANNOTATION_LABEL "!selection"

MidiFileEvent_t MidiFileTrack_createSelectionAnnotationEvent(MidiFileTrack_t track, long tick);
int MidiFileEvent_isSelectionAnnotationEvent(MidiFileEvent_t event);

MidiFileEvent_t MidiFile_getFirstSelection(MidiFile_t midi_file);
int MidiFile_clearSelections(MidiFile_t midi_file);
MidiFileEvent_t MidiFileTrack_getFirstSelection(MidiFileTrack_t track);
int MidiFileTrack_clearSelections(MidiFileTrack_t track);
MidiFileEvent_t MidiFileEvent_getNextSelectionInTrack(MidiFileEvent_t event);
MidiFileEvent_t MidiFileEvent_getNextSelectionInFile(MidiFileEvent_t event);
int MidiFileEvent_isSelected(MidiFileEvent_t event);
int MidiFileEvent_setSelected(MidiFileEvent_t event, int selected);



#define MIDI_FILE_EMPTY_ANNOTATION_TARGET_LABEL "!empty-target"

MidiFileEvent_t MidiFileTrack_createEmptyAnnotationTargetEvent(MidiFileTrack_t track, long tick);
int MidiFileEvent_isEmptyAnnotationTargetEvent(MidiFileEvent_t event);

int MidiFile_deleteOrphanEmptyAnnotationTargetEvents(MidiFile_t midi_file);
int MidiFileEmptyAnnotationTargetEvent_isOrphan(MidiFileEvent_t event);



int MidiFileEvent_deleteWithAnnotations(MidiFileEvent_t event);
int MidiFileEvent_setTrackWithAnnotations(MidiFileEvent_t event, MidiFileTrack_t track);
int MidiFileEvent_setTickWithAnnotations(MidiFileEvent_t event, long tick);


#ifdef __cplusplus
}
#endif

#endif

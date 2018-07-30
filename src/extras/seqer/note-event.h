#ifndef NOTE_EVENT_INCLUDED
#define NOTE_EVENT_INCLUDED

class NoteEventType;
class NoteEventTimeCell;
class NoteEventTrackCell;
class NoteEventChannelCell;
class NoteEventNoteCell;
class NoteEventVelocityCell;
class NoteEventEndTimeCell;
class NoteEventEndVelocityCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class NoteEventType: public EventType
{
public:
	static NoteEventType* GetInstance();

private:
	NoteEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
	void DeleteRow(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventTimeCell: public Cell
{
public:
	NoteEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
	void LargeIncrease(SequenceEditor* sequence_editor, Row* row);
	void LargeDecrease(SequenceEditor* sequence_editor, Row* row);
	void Quantize(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventTrackCell: public Cell
{
public:
	NoteEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventChannelCell: public Cell
{
public:
	NoteEventChannelCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventNoteCell: public Cell
{
public:
	NoteEventNoteCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
	void LargeIncrease(SequenceEditor* sequence_editor, Row* row);
	void LargeDecrease(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventVelocityCell: public Cell
{
public:
	NoteEventVelocityCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
	void LargeIncrease(SequenceEditor* sequence_editor, Row* row);
	void LargeDecrease(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventEndTimeCell: public Cell
{
public:
	NoteEventEndTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
	void LargeIncrease(SequenceEditor* sequence_editor, Row* row);
	void LargeDecrease(SequenceEditor* sequence_editor, Row* row);
	void Quantize(SequenceEditor* sequence_editor, Row* row);
};

class NoteEventEndVelocityCell: public Cell
{
public:
	NoteEventEndVelocityCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
	void SmallIncrease(SequenceEditor* sequence_editor, Row* row);
	void SmallDecrease(SequenceEditor* sequence_editor, Row* row);
	void LargeIncrease(SequenceEditor* sequence_editor, Row* row);
	void LargeDecrease(SequenceEditor* sequence_editor, Row* row);
};

#endif

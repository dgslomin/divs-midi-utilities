#ifndef NOTE_EVENT_INCLUDED
#define NOTE_EVENT_INCLUDED

class NoteEventType;
class NoteEventRow;
class NoteEventTimeCell;
class NoteEventTrackCell;
class NoteEventChannelCell;
class NoteEventNoteCell;
class NoteEventVelocityCell;
class NoteEventEndTimeCell;
class NoteEventEndVelocityCell;

#include <wx/wx.h>
#include <midifile.h>
#include "event-type.h"
#include "sequence-editor.h"

class NoteEventType: public EventType
{
public:
	NoteEventType();
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class NoteEventRow: public Row
{
public:
	NoteEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
	void Delete();
};

class NoteEventTimeCell: public Cell
{
public:
	NoteEventTimeCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
	void LargeIncrease();
	void LargeDecrease();
	void Quantize();
};

class NoteEventTrackCell: public Cell
{
public:
	NoteEventTrackCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
};

class NoteEventChannelCell: public Cell
{
public:
	NoteEventChannelCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
};

class NoteEventNoteCell: public Cell
{
public:
	NoteEventNoteCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
	void LargeIncrease();
	void LargeDecrease();
};

class NoteEventVelocityCell: public Cell
{
public:
	NoteEventVelocityCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
	void LargeIncrease();
	void LargeDecrease();
};

class NoteEventEndTimeCell: public Cell
{
public:
	NoteEventEndTimeCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
	void LargeIncrease();
	void LargeDecrease();
	void Quantize();
};

class NoteEventEndVelocityCell: public Cell
{
public:
	NoteEventEndVelocityCell(Row* row);
	wxString GetValueText();
	void SmallIncrease();
	void SmallDecrease();
	void LargeIncrease();
	void LargeDecrease();
};

#endif

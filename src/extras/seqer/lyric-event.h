#ifndef LYRIC_EVENT_INCLUDED
#define LYRIC_EVENT_INCLUDED

class LyricEventType;
class LyricEventRow;
class LyricEventTimeCell;
class LyricEventTrackCell;
class LyricEventLyricCell;

#include <wx/wx.h>
#include <midifile.h>
#include "event-type.h"
#include "sequence-editor.h"

class LyricEventType: public EventType
{
public:
	LyricEventType();
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class LyricEventRow: public Row
{
public:
	LyricEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class LyricEventTimeCell: public Cell
{
public:
	LyricEventTimeCell(Row* row);
	wxString GetValueText();
};

class LyricEventTrackCell: public Cell
{
public:
	LyricEventTrackCell(Row* row);
	wxString GetValueText();
};

class LyricEventLyricCell: public Cell
{
public:
	LyricEventLyricCell(Row* row);
	wxString GetValueText();
};

#endif

#ifndef LYRIC_EVENT_INCLUDED
#define LYRIC_EVENT_INCLUDED

class LyricEventType;
class LyricEventTimeCell;
class LyricEventTrackCell;
class LyricEventLyricCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class LyricEventType: public EventType
{
public:
	static LyricEventType* GetInstance();

private:
	LyricEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class LyricEventTimeCell: public Cell
{
public:
	LyricEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class LyricEventTrackCell: public Cell
{
public:
	LyricEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class LyricEventLyricCell: public Cell
{
public:
	LyricEventLyricCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif

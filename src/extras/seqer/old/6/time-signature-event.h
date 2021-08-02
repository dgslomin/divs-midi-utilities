#ifndef TIME_SIGNATURE_EVENT_INCLUDED
#define TIME_SIGNATURE_EVENT_INCLUDED

class TimeSignatureEventType;
class TimeSignatureEventRow;
class TimeSignatureEventTimeCell;
class TimeSignatureEventTimeSignatureCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class TimeSignatureEventType: public EventType
{
public:
	static TimeSignatureEventType* GetInstance();

private:
	TimeSignatureEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class TimeSignatureEventRow: public Row
{
public:
	TimeSignatureEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class TimeSignatureEventTimeCell: public Cell
{
public:
	TimeSignatureEventTimeCell(Row* row);
	wxString GetValueText();
};

class TimeSignatureEventTimeSignatureCell: public Cell
{
public:
	TimeSignatureEventTimeSignatureCell(Row* row);
	wxString GetValueText();
};

#endif

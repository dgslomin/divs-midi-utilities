#ifndef SYSTEM_EXCLUSIVE_EVENT_INCLUDED
#define SYSTEM_EXCLUSIVE_EVENT_INCLUDED

class SystemExclusiveEventType;
class SystemExclusiveEventRow;
class SystemExclusiveEventTimeCell;
class SystemExclusiveEventTrackCell;

#include <wx/wx.h>
#include <midifile.h>
#include "event-types.h"
#include "sequence-editor.h"

class SystemExclusiveEventType: public EventType
{
public:
	SystemExclusiveEventType();
	bool MatchesEvent(MidiFileEvent_t event);
	Row* GetRow(MidiFileEvent_t event);
};

class SystemExclusiveEventRow: public Row
{
public:
	SystemExclusiveEventRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

class SystemExclusiveEventTimeCell: public Cell
{
public:
	SystemExclusiveEventTimeCell(Row* row);
	wxString GetValueText();
};

class SystemExclusiveEventTrackCell: public Cell
{
public:
	SystemExclusiveEventTrackCell(Row* row);
	wxString GetValueText();
};

#endif

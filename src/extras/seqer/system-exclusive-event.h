#ifndef SYSTEM_EXCLUSIVE_EVENT_INCLUDED
#define SYSTEM_EXCLUSIVE_EVENT_INCLUDED

class SystemExclusiveEventType;
class SystemExclusiveEventTimeCell;
class SystemExclusiveEventTrackCell;

#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class SystemExclusiveEventType: public EventType
{
public:
	static SystemExclusiveEventType* GetInstance();

private:
	SystemExclusiveEventType();

public:
	bool MatchesEvent(MidiFileEvent_t event);
};

class SystemExclusiveEventTimeCell: public Cell
{
public:
	SystemExclusiveEventTimeCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

class SystemExclusiveEventTrackCell: public Cell
{
public:
	SystemExclusiveEventTrackCell();
	wxString GetValueText(SequenceEditor* sequence_editor, Row* row);
};

#endif

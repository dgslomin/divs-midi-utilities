#ifndef EVENT_TYPES_INCLUDED
#define EVENT_TYPES_INCLUDED

class EventTypes;
class EventType;

#include <vector>
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

class EventTypes
{
public:
	std::vector<EventType> event_types;

	EventTypes();
	EventType* GetEventType(MidiFileEvent_t event);
};

class EventType
{
public:
	wxString name;

	virtual bool MatchesEvent(MidiFileEvent_t event) = 0;
	virtual bool GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event);
};

#endif

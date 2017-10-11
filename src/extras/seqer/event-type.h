#ifndef EVENT_TYPE_INCLUDED
#define EVENT_TYPE_INCLUDED

class EventTypeManager;
class EventType;

#include <vector>
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"

extern EventTypeManager* event_type_manager;

class EventTypeManager
{
public:
	std::vector<EventType*> event_types;

	EventTypeManager();
	~EventTypeManager();
	EventType* GetEventType(MidiFileEvent_t event);
};

class EventType
{
public:
	wxString name;

	virtual ~EventType() = 0;
	virtual bool MatchesEvent(MidiFileEvent_t event) = 0;
	virtual Row* GetRow(SequenceEditor* sequence_editor, long step_number, MidiFileEvent_t event) = 0;
};

#endif

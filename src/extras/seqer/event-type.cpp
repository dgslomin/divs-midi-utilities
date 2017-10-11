
#include <wx/wx.h>
#include <midifile.h>
#include "aftertouch-event.h"
#include "control-change-event.h"
#include "event-type.h"
#include "key-signature-event.h"
#include "lyric-event.h"
#include "marker-event.h"
#include "note-event.h"
#include "pitch-bend-event.h"
#include "port-event.h"
#include "program-change-event.h"
#include "sequence-editor.h"
#include "system-exclusive-event.h"
#include "tempo-event.h"
#include "text-event.h"
#include "time-signature-event.h"

EventTypeManager* event_type_manager = new EventTypeManager();

EventTypeManager::EventTypeManager()
{
	this->event_types.push_back(new NoteEventType());
	this->event_types.push_back(new ControlChangeEventType());
	this->event_types.push_back(new ProgramChangeEventType());
	this->event_types.push_back(new AftertouchEventType());
	this->event_types.push_back(new PitchBendEventType());
	this->event_types.push_back(new SystemExclusiveEventType());
	this->event_types.push_back(new TextEventType());
	this->event_types.push_back(new LyricEventType());
	this->event_types.push_back(new MarkerEventType());
	this->event_types.push_back(new PortEventType());
	this->event_types.push_back(new TempoEventType());
	this->event_types.push_back(new TimeSignatureEventType());
	this->event_types.push_back(new KeySignatureEventType());
}

EventTypeManager::~EventTypeManager()
{
	for (int event_type_number = 0; event_type_number < this->event_types.size(); event_type_number++) delete this->event_types[event_type_number];
}

EventType* EventTypeManager::GetEventType(MidiFileEvent_t event)
{
	for (int event_type_number = 0; event_type_number < this->event_types.size(); event_type_number++)
	{
		EventType* event_type = this->event_types[event_type_number];
		if (event_type->MatchesEvent(event)) return event_type;
	}

	return NULL;
}

EventType::~EventType()
{
}


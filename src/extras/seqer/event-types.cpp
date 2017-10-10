
#include <wx/wx.h>
#include <midifile.h>
#include "aftertouch-event.h"
#include "control-change-event.h"
#include "event-types.h"
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

EventTypes::EventTypes()
{
	this->event_types.push_back(NoteEventType());
	this->event_types.push_back(ControlChangeEventType());
	this->event_types.push_back(ProgramChangeEventType());
	this->event_types.push_back(AftertouchEventType());
	this->event_types.push_back(PitchBendEventType());
	this->event_types.push_back(SystemExclusiveEventType());
	this->event_types.push_back(TextEventType());
	this->event_types.push_back(LyricEventType());
	this->event_types.push_back(MarkerEventType());
	this->event_types.push_back(PortEventType());
	this->event_types.push_back(TempoEventType());
	this->event_types.push_back(TimeSignatureEventType());
	this->event_types.push_back(KeySignatureEventType());
}

EventType* EventTypes::GetEventType(MidiFileEvent_t event)
{
	for (int event_type_number = 0; event_type_number < this->event_types.size(); event_type_number++)
	{
		EventType* event_type = this->event_types[event_type_number];
		if (event_type->MatchesEvent(event)) return event_type;
	}

	return NULL;
}


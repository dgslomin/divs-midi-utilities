#ifndef EVENT_TYPE_INCLUDED
#define EVENT_TYPE_INCLUDED

#include <wx/wx.h>
#include <midifile.h>

class EventType;
class EventList;

extern EventType* event_types[];

class EventType
{
public:
	virtual wxString GetName() = 0;
	virtual bool Matches(MidiFileEvent_t event) = 0;
	virtual wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number) = 0;
};

class NoteEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class ControlChangeEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class ProgramChangeEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class AftertouchEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class PitchBendEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class SystemExclusiveEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class TextEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class LyricEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class MarkerEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class PortEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class TempoEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class TimeSignatureEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

class KeySignatureEventType: public EventType
{
public:
	wxString GetName();
	bool Matches(MidiFileEvent_t event);
	wxString GetEventListColumnText(EventList* event_list, MidiFileEvent_t event, int column_number);
};

#endif

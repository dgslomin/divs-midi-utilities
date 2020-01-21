
#include <wx/wx.h>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "velocity-lane.h"
#include "window.h"

VelocityLane::VelocityLane(Window* window): NumericValueLane(window)
{
}

VelocityLane::~VelocityLane()
{
}

bool VelocityLane::ShouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return MidiFileEvent_isNoteStartEvent(midi_event);
}

MidiFileEvent_t VelocityLane::AddEvent(long WXUNUSED(tick), float WXUNUSED(value))
{
	// You can't add a velocity, only change the velocity of an existing note.
	return NULL;
}

float VelocityLane::GetEventValue(MidiFileEvent_t midi_event)
{
	return (float)(MidiFileNoteStartEvent_getVelocity(midi_event));
}

void VelocityLane::SetEventValue(MidiFileEvent_t midi_event, float value)
{
	MidiFileNoteStartEvent_setVelocity(midi_event, (int)(value));
}



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

int VelocityLane::GetValueFromEvent(MidiFileEvent_t midi_event)
{
	return MidiFileNoteStartEvent_getVelocity(midi_event);
}


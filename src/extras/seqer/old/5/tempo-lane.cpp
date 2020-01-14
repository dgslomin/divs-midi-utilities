
#include <wx/wx.h>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "tempo-lane.h"
#include "window.h"

TempoLane::TempoLane(Window* window): NumericValueLane(window)
{
}

TempoLane::~TempoLane()
{
}

bool TempoLane::ShouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return MidiFileEvent_isTempoEvent(midi_event);
}

int TempoLane::GetValueFromEvent(MidiFileEvent_t midi_event)
{
	return MidiFileTempoEvent_getTempo(midi_event);
}


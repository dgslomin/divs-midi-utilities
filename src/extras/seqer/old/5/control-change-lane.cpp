
#include <wx/wx.h>
#include "control-change-lane.h"
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

ControlChangeLane::ControlChangeLane(Window* window): NumericValueLane(window)
{
}

ControlChangeLane::~ControlChangeLane()
{
}

bool ControlChangeLane::ShouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) && (MidiFileControlChangeEvent_getNumber(midi_event) == this->controller_number);
}

int ControlChangeLane::GetValueFromEvent(MidiFileEvent_t midi_event)
{
	return MidiFileControlChangeEvent_getValue(midi_event);
}



#include <wx/wx.h>
#include "controller-lane.h"
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

ControllerLane::ControllerLane(wxWindow* parent, Window* window): NumericValueLane(parent, window)
{
}

ControllerLane::~ControllerLane()
{
}

bool ControllerLane::ShouldIncludeEvent(MidiFileEvent_t midi_event)
{
	return (MidiFileEvent_getType(midi_event) == MIDI_FILE_EVENT_TYPE_CONTROL_CHANGE) && (MidiFileControlChangeEvent_getNumber(midi_event) == this->controller_number);
}

MidiFileEvent_t ControllerLane::AddEvent(long tick, float value)
{
	return MidiFileTrack_createControlChangeEvent(this->track, tick, this->channel, this->controller_number, (int)(value));
}

float ControllerLane::GetEventValue(MidiFileEvent_t midi_event)
{
	return (float)(MidiFileControlChangeEvent_getValue(midi_event));
}

void ControllerLane::SetEventValue(MidiFileEvent_t midi_event, float value)
{
	MidiFileControlChangeEvent_setValue(midi_event, (int)(value));
}


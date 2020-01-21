#ifndef CONTROL_CHANGE_LANE_INCLUDED
#define CONTROL_CHANGE_LANE_INCLUDED

class ControlChangeLane;

#include <wx/wx.h>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

class ControlChangeLane: public NumericValueLane
{
public:
	MidiFileTrack_t track;
	int channel;
	int controller_number;

	ControlChangeLane(Window* window);
	~ControlChangeLane();
	virtual bool ShouldIncludeEvent(MidiFileEvent_t midi_event);
	virtual MidiFileEvent_t AddEvent(long tick, float value);
	virtual float GetEventValue(MidiFileEvent_t midi_event);
	virtual void SetEventValue(MidiFileEvent_t midi_event, float value);
};

#endif

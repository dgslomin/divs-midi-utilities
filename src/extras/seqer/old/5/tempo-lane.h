#ifndef VELOCITY_LANE_INCLUDED
#define VELOCITY_LANE_INCLUDED

class TempoLane;

#include <wx/wx.h>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

class TempoLane: public NumericValueLane
{
public:
	int pixels_per_value;
	int scroll_y;

	TempoLane(Window* window);
	~TempoLane();
	virtual bool ShouldIncludeEvent(MidiFileEvent_t midi_event);
	virtual int GetValueFromEvent(MidiFileEvent_t midi_event);
};

#endif

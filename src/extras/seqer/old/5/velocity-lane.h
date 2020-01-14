#ifndef VELOCITY_LANE_INCLUDED
#define VELOCITY_LANE_INCLUDED

class VelocityLane;

#include <wx/wx.h>
#include "midifile.h"
#include "numeric-value-lane.h"
#include "window.h"

class VelocityLane: public NumericValueLane
{
public:
	int pixels_per_value;
	int scroll_y;

	VelocityLane(Window* window);
	~VelocityLane();
	virtual bool ShouldIncludeEvent(MidiFileEvent_t midi_event);
	virtual int GetValueFromEvent(MidiFileEvent_t midi_event);
};

#endif

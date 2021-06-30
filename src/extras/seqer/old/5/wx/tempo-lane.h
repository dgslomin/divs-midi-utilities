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

	TempoLane(wxWindow* parent, Window* window);
	~TempoLane();
	virtual bool ShouldIncludeEvent(MidiFileEvent_t midi_event);
	virtual MidiFileEvent_t AddEvent(long tick, float value);
	virtual float GetEventValue(MidiFileEvent_t midi_event);
	virtual void SetEventValue(MidiFileEvent_t midi_event, float value);
};

#endif

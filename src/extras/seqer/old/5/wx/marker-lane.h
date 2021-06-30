#ifndef MARKER_LANE_INCLUDED
#define MARKER_LANE_INCLUDED

class MarkerLane;

#include "label-lane.h"
#include "window.h"

class MarkerLane: public LabelLane
{
public:
	MarkerLane(wxWindow* parent, Window* window);
	~MarkerLane();
	virtual void PopulateLabels();
	virtual MidiFileEvent_t AddEvent(long tick);
};

#endif

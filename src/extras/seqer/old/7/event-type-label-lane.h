#ifndef EVENT_TYPE_LABEL_LANE_INCLUDED
#define EVENT_TYPE_LABEL_LANE_INCLUDED

class EventTypeLabelLane;

#include "label-lane.h"
#include "window.h"

class EventTypeLabelLane: public LabelLane
{
public:
	EventTypeLabelLane(wxWindow* parent, Window* window);
	~EventTypeLabelLane();
	virtual void PopulateLabels();
	virtual MidiFileEvent_t AddEvent(long tick);
};

#endif

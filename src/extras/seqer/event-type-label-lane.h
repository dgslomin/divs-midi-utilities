#ifndef EVENT_TYPE_LABEL_LANE_INCLUDED
#define EVENT_TYPE_LABEL_LANE_INCLUDED

class EventTypeLabelLane;

#include "label-lane.h"
#include "midifile.h"
#include "window.h"

class EventTypeLabelLane: public LabelLane
{
public:
	EventTypeLabelLane(Window* window);
	void populateLabels();
	MidiFileEvent_t addEventAtXY(int x, int y);
	void moveEventsByXY(int x_offset, int y_offset);
};

#endif

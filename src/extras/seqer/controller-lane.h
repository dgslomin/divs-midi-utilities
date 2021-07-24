#ifndef CONTROLLER_LANE_INCLUDED
#define CONTROLLER_LANE_INCLUDED

class ControllerLane;

#include <QtWidgets>
#include "numeric-value-lane.h"
#include "midifile.h"
#include "window.h"

class ControllerLane: public NumericValueLane
{
	Q_OBJECT

public:
	int controller_number = 64;

	ControllerLane(Window* window);
	bool shouldIncludeEvent(MidiFileEvent_t midi_event);
	MidiFileEvent_t addEvent(long tick, float value);
	float getEventValue(MidiFileEvent_t midi_event);
	void setEventValue(MidiFileEvent_t midi_event, float value);
};

#endif

#ifndef TEMPO_LANE_INCLUDED
#define TEMPO_LANE_INCLUDED

class TempoLane;

#include <QtWidgets>
#include "numeric-value-lane.h"
#include "midifile.h"
#include "window.h"

class TempoLane: public NumericValueLane
{
	Q_OBJECT

public:
	TempoLane(Window* window);
	void paintValueLines(QPainter* painter);
	bool shouldIncludeEvent(MidiFileEvent_t midi_event);
	MidiFileEvent_t addEvent(long tick, float value);
	float getEventValue(MidiFileEvent_t midi_event);
	void setEventValue(MidiFileEvent_t midi_event, float value);
};

#endif

#ifndef LABEL_LANE_INCLUDED
#define LABEL_LANE_INCLUDED

class LabelLane;

#include "midifile.h"
#include "window.h"

class LabelLane: public Lane
{
	Q_OBJECT

public:
	std::vector<Label> labels;

	LabelLane(Window* window);
	void paintBackground(QPainter* painter);
	void paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset);
	MidiFileEvent_t getEventFromXY(int x, int y);
	QPoint getPointFromEvent(MidiFileEvent_t midi_event);
	MidiFileEvent_t addEventAtXY(int x, int y);
	void moveEventsByXY(int x_offset, int y_offset);
	void selectEventsInRect(int x, int y, int width, int height);
	void scrollYBy(int y_offset);
	void zoomYBy(int y_offset);
	void layoutLabels(int selected_events_x_offset);

	virtual void populateLabels() = 0;
	virtual MidiFileEvent_t addEventAtTick(long tick) = 0;
};

#endif


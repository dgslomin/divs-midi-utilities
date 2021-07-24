#ifndef VELOCITY_LANE_INCLUDED
#define VELOCITY_LANE_INCLUDED

class VelocityLane;

#include <QtWidgets>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class VelocityLane: public Lane
{
	Q_OBJECT

public:
	float pixels_per_value = 1.0;
	float scroll_y = 0.0;
	int box_width;

	VelocityLane(Window* window);
	void paintBackground(QPainter* painter);
	void paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset);
	MidiFileEvent_t getEventFromXY(int x, int y);
	QPoint getPointFromEvent(MidiFileEvent_t midi_event);
	MidiFileEvent_t addEventAtXY(int x, int y);
	void moveEventsByXY(int x_offset, int y_offset);
	void selectEventsInRect(int x, int y, int width, int height);
	void scrollYBy(int offset);
	void zoomYBy(float factor);
	int getCursorGap();
	QRect getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset);
	int getYFromValue(float value);
	float getValueFromY(int y);
};

#endif

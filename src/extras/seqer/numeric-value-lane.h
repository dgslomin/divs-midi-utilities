#ifndef NUMERIC_VALUE_LANE_INCLUDED
#define NUMERIC_VALUE_LANE_INCLUDED

class NumericValueLane;

#include <QtWidgets>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class NumericValueLane: public Lane
{
	Q_OBJECT

public:
	float pixels_per_value = 1.0;
	float scroll_y = 0.0;
	bool draw_connecting_lines = false;
	bool draw_height_lines = false;
	int handle_size;
	QPen connecting_line_pen;
	QPen height_line_pen;

	NumericValueLane(Window* window);
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
	virtual void paintValueLines(QPainter* painter) = 0;
	virtual bool shouldIncludeEvent(MidiFileEvent_t midi_event) = 0;
	virtual MidiFileEvent_t addEvent(long tick, float value) = 0;
	virtual float getEventValue(MidiFileEvent_t midi_event) = 0;
	virtual void setEventValue(MidiFileEvent_t midi_event, float value) = 0;
};

#endif


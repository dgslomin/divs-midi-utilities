#ifndef LABEL_LANE_INCLUDED
#define LABEL_LANE_INCLUDED

class LabelLane;
class Label;

#include <QtWidgets>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class LabelLane: public Lane
{
	Q_OBJECT

public:
	QVector<Label> labels;

	LabelLane(Window* window);
	void paintBackground(QPainter* painter);
	void paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset);
	MidiFileEvent_t getEventFromXY(int x, int y);
	QPoint getPointFromEvent(MidiFileEvent_t midi_event);
	void selectEventsInRect(int x, int y, int width, int height);
	void scrollYBy(int offset);
	void zoomYBy(float factor);
	void computeLabelWidths();
	void layoutLabels(int selected_events_x_offset);

	virtual void populateLabels() = 0;
	virtual MidiFileEvent_t addEventAtXY(int x, int y) = 0;
	virtual void moveEventsByXY(int x_offset, int y_offset) = 0;
};

class Label
{
public:
	MidiFileEvent_t midi_event;
	QString text;
	int preferred_x;
	int width;
	int x;
	int row;

	Label(MidiFileEvent_t midi_event = NULL, QString text = NULL);
};

#endif

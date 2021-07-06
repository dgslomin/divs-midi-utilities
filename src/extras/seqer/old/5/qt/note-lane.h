#ifndef NOTE_LANE_INCLUDED
#define NOTE_LANE_INCLUDED

class NoteLane;

#include <QPainter>
#include <QRect>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class NoteLane: public Lane
{
	Q_OBJECT

public:
	int channel = 0;
	int velocity = 0;
	int pixels_per_note;
	int scroll_y;

	NoteLane(Window* window);
	void paintBackground(QPainter* painter);
	void paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset);
	MidiFileEvent_t getEventFromXY(int x, int y);
	MidiFileEvent_t addEventAtXY(int x, int y);
	void moveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset);
	void selectEventsInRect(int x, int y, int width, int height);
	QRect getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset);
	int getYFromNote(int note);
	int getNoteFromY(int y);
};

#endif

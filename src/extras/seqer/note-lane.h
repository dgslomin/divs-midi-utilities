#ifndef NOTE_LANE_INCLUDED
#define NOTE_LANE_INCLUDED

class NoteLane;

#include <QtWidgets>
#include "lane.h"
#include "midifile.h"
#include "window.h"

enum NoteLaneDragOrigin
{
	NOTE_LANE_DRAG_ORIGIN_NONE,
	NOTE_LANE_DRAG_ORIGIN_START,
	NOTE_LANE_DRAG_ORIGIN_MIDDLE,
	NOTE_LANE_DRAG_ORIGIN_END
};

class NoteLane: public Lane
{
	Q_OBJECT

public:
	int velocity = 63;
	NoteLaneDragOrigin drag_origin = NOTE_LANE_DRAG_ORIGIN_NONE;
	float pixels_per_note;
	float scroll_y;
	QColor white_note_background_color;
	QColor black_note_background_color;
	int handle_size;

	NoteLane(Window* window);
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
	void setDragOriginFromXY(int x, int y);
	QRect getRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset);
	int getYFromNote(int note);
	int getNoteFromY(int y);
};

#endif

#ifndef LANE_INCLUDED
#define LANE_INCLUDED

class Lane;

#include <QColor>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>
#include "midifile.h"
#include "window.h"

class Lane: public QWidget
{
	Q_OBJECT

public:
	Window* window;
	MidiFileTrack_t track;
	int cursor_x = 0;
	int cursor_y = 0;
	bool mouse_down = false;
	int mouse_down_x = 0;
	int mouse_down_y = 0;
	MidiFileEvent_t mouse_down_midi_event = NULL;
	bool mouse_down_midi_event_is_new = false;
	int mouse_drag_x = 0;
	int mouse_drag_y = 0;
	bool mouse_drag_x_allowed = false;
	bool mouse_drag_y_allowed = false;
	QColor background_color;
	QColor white_note_background_color;
	QColor black_note_background_color;
	QPen unselected_event_pen;
	QBrush unselected_event_brush;
	QPen unselected_event_text_pen;
	QPen selected_event_pen;
	QBrush selected_event_brush;
	QPen selected_event_text_pen;
	QPen cursor_pen;
	QPen selection_rect_pen;
	int mouse_drag_threshold;

	Lane(Window* window);
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

	virtual void paintBackground(QPainter* painter) = 0;
	virtual void paintEvents(QPainter* painter, int selected_events_x_offset, int selected_events_y_offset) = 0;
	virtual MidiFileEvent_t getEventFromXY(int x, int y) = 0;
	virtual MidiFileEvent_t addEventAtXY(int x, int y) = 0;
	virtual void moveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset) = 0;
	virtual void selectEventsInRect(int x, int y, int width, int height) = 0;

public slots:
	void editEvent();
	void selectEvent();
	void toggleEventSelection();
	void cursorLeft();
	void cursorRight();
	void cursorUp();
	void cursorDown();
};

#endif

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
	int cursor_x;
	int cursor_y;
	bool mouse_down;
	int mouse_down_x;
	int mouse_down_y;
	MidiFileEvent_t mouse_down_midi_event;
	bool mouse_down_midi_event_is_new;
	int mouse_drag_x;
	int mouse_drag_y;
	bool mouse_drag_x_allowed;
	bool mouse_drag_y_allowed;
	QColor background_color;
	QColor white_note_background_color;
	QColor black_note_background_color;
	QColor unselected_event_color;
	QColor unselected_event_border_color;
	QColor unselected_event_text_color;
	QColor selected_event_color;
	QColor selected_event_border_color;
	QColor selected_event_text_color;
	QColor cursor_color;
	QColor selection_rect_color;
	QColor selection_rect_border_color;
	int mouse_drag_threshold;

	Lane(Window* window);
	~Lane();

	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

	virtual void paintBackground(QPainter* painter, int width, int height) = 0;
	virtual void paintEvents(QPainter* painter, int width, int height, int selected_events_x_offset, int selected_events_y_offset) = 0;
	virtual MidiFileEvent_t getEventFromXY(int x, int y) = 0;
	virtual MidiFileEvent_t addEventAtXY(int x, int y) = 0;
	virtual void moveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset) = 0;
	virtual void selectEventsInRect(int x, int y, int width, int height) = 0;

public slots:
	void editEvent();
	void selectEvent();
	void toggleEventSelection();
};

#endif

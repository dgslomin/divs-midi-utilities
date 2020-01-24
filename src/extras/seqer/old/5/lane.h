#ifndef LANE_INCLUDED
#define LANE_INCLUDED

class Lane;

#include <wx/wx.h>
#include "midifile.h"
#include "window.h"

class Lane: public wxWindow
{
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

	Lane(wxWindow* parent, Window* window);
	~Lane();
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnLeftDown(wxMouseEvent& event);
	virtual void OnLeftUp(wxMouseEvent& event);
	virtual void OnMotion(wxMouseEvent& event);
	virtual void OnChar(wxKeyEvent& event);
	virtual void OnReturnChar(wxKeyEvent& event);
	virtual void OnSpaceChar(wxKeyEvent& event);

	virtual void PaintBackground(wxDC& dc, int width, int height) = 0;
	virtual void PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int selected_events_y_offset) = 0;
	virtual MidiFileEvent_t GetEventFromXY(int x, int y) = 0;
	virtual MidiFileEvent_t AddEventAtXY(int x, int y) = 0;
	virtual void MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset) = 0;
	virtual void SelectEventsInRect(int x, int y, int width, int height) = 0;
};

#endif

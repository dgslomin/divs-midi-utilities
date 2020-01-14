#ifndef NOTE_LANE_INCLUDED
#define NOTE_LANE_INCLUDED

class ControlChaneLane;

#include <wx/wx.h>
#include "lane.h"
#include "window.h"

class NoteLane: public Lane
{
public:
	int pixels_per_note;
	int scroll_y;
	wxBrush white_note_background_brush;
	wxBrush black_note_background_brush;

	NoteLane(Window* window);
	~NoteLane();
	virtual void OnPaint(wxPaintEvent& event);
	void PaintBackground(wxDC& dc, int width, int height);
	void PaintNotes(wxDC& dc, int width, int height);
	virtual int GetYFromNote(int note);
	virtual int GetNoteFromY(int y);
};

#endif


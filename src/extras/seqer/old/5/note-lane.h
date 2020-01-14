#ifndef NOTE_LANE_INCLUDED
#define NOTE_LANE_INCLUDED

class ControlChaneLane;

#include <wx/wx.h>
#include "lane.h"
#include "window.h"

class NoteLane: public Lane
{
public:
	NoteLane(Window* window): Lane(window);
	~NoteLane();
	virtual void OnPaint(wxPaintEvent& event);
	void PaintBackground(wxDC& dc);
	void PaintNotes(wxDC& dc);
	int GetYForNote(int note);
	int GetNoteForY(int y);
};

#endif


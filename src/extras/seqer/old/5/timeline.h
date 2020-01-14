#ifndef TIMELINE_INCLUDED
#define TIMELINE_INCLUDED

class Timeline;

#include <wx/wx.h>
#include "window.h"

class Timeline: public wxWindow
{
public:
	Window* window;

	Timeline(Window* window);
	~Timeline();
	virtual void OnPaint(wxPaintEvent& event);
	virtual void PaintMeasureBeatTimeline(wxDC& dc, long min_tick, long max_tick, int y);
	virtual void PaintHourMinuteSecondTimeline(wxDC& dc, long min_tick, long max_tick, int y);
};

#endif

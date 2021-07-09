#ifndef TIME_RULER_INCLUDED
#define TIME_RULER_INCLUDED

class TimeRuler;

#include <wx/wx.h>
#include "window.h"

class TimeRuler: public wxWindow
{
public:
	Window* window;

	TimeRuler(Window* window);
	~TimeRuler();
	virtual void OnPaint(wxPaintEvent& event);
	virtual void PaintMeasureBeatRuler(wxDC& dc, long min_tick, long max_tick, int y);
	virtual void PaintHourMinuteSecondRuler(wxDC& dc, long min_tick, long max_tick, int y);
};

#endif

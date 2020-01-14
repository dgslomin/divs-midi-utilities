#ifndef TIMELINE_INCLUDED
#define TIMELINE_INCLUDED

class Timeline;

#include <wx/wx.h>
#include "window.h"

class Timeline: public wxWindow
{
public:
	Timeline(Window* window);
	~Timeline();
	virtual void OnPaint(wxPaintEvent& event);
};

#endif

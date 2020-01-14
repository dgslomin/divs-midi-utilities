#ifndef LANE_INCLUDED
#define LANE_INCLUDED

class Lane;

#include <wx/wx.h>
#include "window.h"

class Lane: public wxWindow
{
public:
	Window* window;

	Lane(Window* window);
	~Lane();
	virtual void OnPaint(wxPaintEvent& event);
};

#endif

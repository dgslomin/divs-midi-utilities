#ifndef LANE_INCLUDED
#define LANE_INCLUDED

class Lane;

#include <wx/wx.h>
#include "window.h"

class Lane: public wxWindow
{
public:
	Lane(Window* window): wxWindow(window, wxID_ANY);
	~Lane();
	virtual void OnPaint(wxPaintEvent& event);
};

#endif

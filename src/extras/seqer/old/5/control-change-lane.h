#ifndef CONTROL_CHANGE_LANE_INCLUDED
#define CONTROL_CHANGE_LANE_INCLUDED

class ControlChangeLane;

#include <wx/wx.h>
#include "lane.h"
#include "window.h"

class ControlChangeLane: public Lane
{
public:
	ControlChangeLane(Window* window): Lane(window);
	~ControlChangeLane();
	virtual void OnPaint(wxPaintEvent& event);
	int GetYForValue(int value);
	int GetValueForY(int y);
};

#endif

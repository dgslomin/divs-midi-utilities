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

	Lane(Window* window);
	~Lane();
	virtual void OnPaint(wxPaintEvent& event) = 0;
	virtual MidiFileEvent_t GetEventFromXY(int x, int y) = 0;
};

#endif

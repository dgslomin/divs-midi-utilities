#ifndef CONTROL_CHANGE_LANE_INCLUDED
#define CONTROL_CHANGE_LANE_INCLUDED

class ControlChangeLane;

#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class ControlChangeLane: public Lane
{
public:
	int controller_number;
	int pixels_per_value;
	int scroll_y;

	ControlChangeLane(Window* window);
	~ControlChangeLane();
	virtual void OnPaint(wxPaintEvent& event);
	virtual MidiFileEvent_t GetEventFromXY(int x, int y);
	virtual wxRect GetRectFromEvent(MidiFileEvent_t midi_event);
	virtual int GetYFromValue(int value);
	virtual int GetValueFromY(int y);
};

#endif

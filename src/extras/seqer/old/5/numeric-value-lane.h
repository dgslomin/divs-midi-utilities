#ifndef NUMERIC_VALUE_LANE_INCLUDED
#define NUMERIC_VALUE_LANE_INCLUDED

class NumericValueLane;

#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class NumericValueLane: public Lane
{
public:
	int pixels_per_value;
	int scroll_y;

	NumericValueLane(Window* window);
	~NumericValueLane();
	virtual void OnPaint(wxPaintEvent& event);
	virtual MidiFileEvent_t GetEventFromXY(int x, int y);
	virtual wxRect GetRectFromEvent(MidiFileEvent_t midi_event);
	virtual int GetYFromValue(int value);
	virtual int GetValueFromY(int y);
	virtual bool ShouldIncludeEvent(MidiFileEvent_t midi_event) = 0;
	virtual int GetValueFromEvent(MidiFileEvent_t midi_event) = 0;
};

#endif

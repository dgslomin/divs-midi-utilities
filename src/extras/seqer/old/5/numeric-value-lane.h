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

	NumericValueLane(wxWindow* parent, Window* window);
	~NumericValueLane();
	virtual void PaintBackground(wxDC& dc, int width, int height);
	virtual void PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int selected_events_y_offset);
	virtual MidiFileEvent_t GetEventFromXY(int x, int y);
	virtual MidiFileEvent_t AddEventAtXY(int x, int y);
	virtual void MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset);
	virtual void SelectEventsInRect(int x, int y, int width, int height);
	virtual wxRect GetRectFromEvent(MidiFileEvent_t midi_event, int selected_events_x_offset, int selected_events_y_offset);
	virtual int GetYFromValue(float value);
	virtual float GetValueFromY(int y);
	virtual bool ShouldIncludeEvent(MidiFileEvent_t midi_event) = 0;
	virtual MidiFileEvent_t AddEvent(long tick, float value) = 0;
	virtual float GetEventValue(MidiFileEvent_t midi_event) = 0;
	virtual void SetEventValue(MidiFileEvent_t midi_event, float value) = 0;
};

#endif

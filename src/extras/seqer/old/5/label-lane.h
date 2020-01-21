#ifndef LABEL_LANE_INCLUDED
#define LABEL_LANE_INCLUDED

class LabelLane;

#include <vector>
#include <wx/wx.h>
#include "label.h"
#include "lane.h"
#include "window.h"

class LabelLane: public Lane
{
public:
	std::vector<Label> labels;

	LabelLane(Window* window);
	~LabelLane();
	virtual void PaintBackground(wxDC& dc, int width, int height);
	virtual void PaintEvents(wxDC& dc, int width, int height, int selected_events_x_offset, int selected_events_y_offset);
	virtual MidiFileEvent_t GetEventFromXY(int x, int y);
	virtual MidiFileEvent_t AddEventAtXY(int x, int y);
	virtual void MoveEventByXY(MidiFileEvent_t midi_event, int x_offset, int y_offset);
	virtual void SelectEventsInRect(int x, int y, int width, int height);
	virtual void LayoutLabels(int selected_events_x_offset);

	virtual void PopulateLabels() = 0;
	virtual MidiFileEvent_t AddEvent(long tick) = 0;
};

#endif

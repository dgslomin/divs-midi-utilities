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
	virtual void OnPaint(wxPaintEvent& event);
	virtual void PopulateLabels();
	virtual void LayoutLabels();
};

#endif

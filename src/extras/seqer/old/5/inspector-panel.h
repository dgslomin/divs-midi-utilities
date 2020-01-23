#ifndef INSPECTOR_PANEL_INCLUDED
#define INSPECTOR_PANEL_INCLUDED

class InspectorPanel;

#include <wx/wx.h>
#include <wx/vscroll.h>
#include "window.h"

class InspectorPanel: public wxVScrolledWindow
{
public:
	Window* window;
	wxChoice* lane_type_choice;
	wxSizerItem* lane_controller_sizer_item;
	wxChoice* lane_controller_choice;
	wxCheckBox* start_lane_group_check_box;

	InspectorPanel(Window* window, wxWindow* parent);
	~InspectorPanel();
	virtual void OnLaneTypeChoice(wxCommandEvent& event);
	virtual void RefreshDisplay();
};

#endif

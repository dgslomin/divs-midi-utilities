#ifndef LANE_PROPERTIES_DIALOG_INCLUDED
#define LANE_PROPERTIES_DIALOG_INCLUDED

class LanePropertiesDialog;

#include <wx/wx.h>
#include "window.h"

class LanePropertiesDialog: public wxDialog
{
public:
	Window* window;
	bool should_add_lane;
	wxChoice* lane_type_choice;
	wxChoice* controller_choice;
	wxCheckBox* start_lane_group_check_box;

	LanePropertiesDialog(Window* window, bool should_add_lane);
	~LanePropertiesDialog();
	virtual void OnLaneTypeChoice(wxCommandEvent& event);
	virtual void OnClose(wxCloseEvent& event);
};

#endif

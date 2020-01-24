
#include <wx/wx.h>
#include <wx/vscroll.h>
#include "window.h"
#include "inspector-panel.h"

InspectorPanel::InspectorPanel(wxWindow* parent, Window* window): wxVScrolledWindow(parent)
{
	this->window = window;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	outer_sizer->Add(new wxStaticText(this, wxID_ANY, "Lane"), wxSizerFlags().Border());
	wxBoxSizer* lane_properties_sizer = new wxBoxSizer(wxVERTICAL);
	outer_sizer->Add(lane_properties_sizer, wxSizerFlags().Border(wxLEFT, wxSizerFlags::GetDefaultBorder() * 5).Expand());

	wxBoxSizer* lane_type_sizer = new wxBoxSizer(wxHORIZONTAL);
	lane_properties_sizer->Add(lane_type_sizer, wxSizerFlags().Expand());
	lane_type_sizer->Add(new wxStaticText(this, wxID_ANY, "Type"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	this->lane_type_choice = new wxChoice(this, wxID_ANY);
	this->lane_type_choice->Append("Note lane");
	this->lane_type_choice->Append("Velocity lane");
	this->lane_type_choice->Append("Controller lane");
	this->lane_type_choice->Append("Tempo lane");
	this->lane_type_choice->Append("Lyric lane");
	this->lane_type_choice->Append("Marker lane");
	this->lane_type_choice->Append("Event type lane");
	lane_type_sizer->Add(this->lane_type_choice, wxSizerFlags().Proportion(1).Align(wxALIGN_CENTER_VERTICAL).Border());

	wxBoxSizer* lane_controller_sizer = new wxBoxSizer(wxHORIZONTAL);
	this->lane_controller_sizer_item = lane_properties_sizer->Add(lane_controller_sizer, wxSizerFlags().Expand());
	lane_controller_sizer->Add(new wxStaticText(this, wxID_ANY, "Controller"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	this->lane_controller_choice = new wxChoice(this, wxID_ANY);
	this->lane_controller_choice->Append("Modulation (2)");
	this->lane_controller_choice->Append("Breath (3)");
	this->lane_controller_choice->Append("Portamento time (6)");
	this->lane_controller_choice->Append("Volume (8)");
	this->lane_controller_choice->Append("Balance (9)");
	this->lane_controller_choice->Append("Pan (11)");
	this->lane_controller_choice->Append("Expression (12)");
	this->lane_controller_choice->Append("Sustain (65)");
	this->lane_controller_choice->Append("Portamento on/off (66)");
	this->lane_controller_choice->Append("Sustenuto (67)");
	this->lane_controller_choice->Append("Soft pedal (68)");
	this->lane_controller_choice->Append("Resonance (71)");
	this->lane_controller_choice->Append("Cutoff (72)");
	this->lane_controller_choice->Append("Release (73)");
	this->lane_controller_choice->Append("Attack (74)");
	this->lane_controller_choice->Append("Portamento amount (85)");
	this->lane_controller_choice->Append("Reverb (92)");
	this->lane_controller_choice->Append("Tremolo (93)");
	this->lane_controller_choice->Append("Chorus (94)");
	this->lane_controller_choice->Append("Detune (95)");
	this->lane_controller_choice->Append("Phaser (96)");
	for (int i = 1; i <= 128; i++) this->lane_controller_choice->Append(wxString::Format("%d", i));
	lane_controller_sizer->Add(this->lane_controller_choice, wxSizerFlags().Proportion(1).Align(wxALIGN_CENTER_VERTICAL).Border());

	wxBoxSizer* start_lane_group_sizer = new wxBoxSizer(wxHORIZONTAL);
	lane_properties_sizer->Add(start_lane_group_sizer, wxSizerFlags().Expand());
	this->start_lane_group_check_box = new wxCheckBox(this, wxID_ANY, "Start lane group");
	start_lane_group_sizer->Add(this->start_lane_group_check_box, wxSizerFlags().Proportion(1).Border(wxTOP | wxRIGHT | wxBOTTOM));

	outer_sizer->Add(new wxStaticText(this, wxID_ANY, "Event"), wxSizerFlags().Border());
	wxBoxSizer* event_properties_sizer = new wxBoxSizer(wxVERTICAL);
	outer_sizer->Add(event_properties_sizer, wxSizerFlags().Border(wxLEFT, wxSizerFlags::GetDefaultBorder() * 5).Expand());

	this->RefreshDisplay();

	this->lane_type_choice->Bind(wxEVT_CHOICE, [=](wxCommandEvent& event) { this->OnLaneTypeChoice(event); });
}

InspectorPanel::~InspectorPanel()
{
}

void InspectorPanel::OnLaneTypeChoice(wxCommandEvent& event)
{
	this->RefreshDisplay();
	event.Skip();
}

void InspectorPanel::RefreshDisplay()
{
	wxString lane_type = this->lane_type_choice->GetString(this->lane_type_choice->GetSelection());

	if (lane_type.IsSameAs("Controller lane"))
	{
		this->lane_controller_sizer_item->Show(true);
	}
	else
	{
		this->lane_controller_sizer_item->Show(false);
	}

	this->Layout();
}


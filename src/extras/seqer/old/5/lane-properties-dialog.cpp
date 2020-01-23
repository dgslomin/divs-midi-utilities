
#include <wx/wx.h>
#include "window.h"
#include "lane-properties-dialog.h"

LanePropertiesDialog::LanePropertiesDialog(Window* window, bool should_add_lane): wxDialog(window, wxID_ANY, "Lane Properties")
{
	this->window = window;
	this->should_add_lane = should_add_lane;

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(outer_sizer);

	wxFlexGridSizer* properties_sizer = new wxFlexGridSizer(2, wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder());
	properties_sizer->AddGrowableCol(1, 1);
	outer_sizer->Add(properties_sizer, wxSizerFlags().Proportion(1).Expand().DoubleBorder());

	properties_sizer->Add(new wxStaticText(this, wxID_ANY, "Type"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->lane_type_choice = new wxChoice(this, wxID_ANY);
	this->lane_type_choice->Append("Note lane");
	this->lane_type_choice->Append("Velocity lane");
	this->lane_type_choice->Append("Controller lane");
	this->lane_type_choice->Append("Tempo lane");
	this->lane_type_choice->Append("Lyric lane");
	this->lane_type_choice->Append("Marker lane");
	this->lane_type_choice->Append("Event type lane");
	properties_sizer->Add(this->lane_type_choice, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));

	properties_sizer->Add(new wxStaticText(this, wxID_ANY, "Controller"), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	this->controller_choice = new wxChoice(this, wxID_ANY);
	this->controller_choice->Append("Modulation (2)");
	this->controller_choice->Append("Breath (3)");
	this->controller_choice->Append("Portamento time (6)");
	this->controller_choice->Append("Volume (8)");
	this->controller_choice->Append("Balance (9)");
	this->controller_choice->Append("Pan (11)");
	this->controller_choice->Append("Expression (12)");
	this->controller_choice->Append("Sustain (65)");
	this->controller_choice->Append("Portamento on/off (66)");
	this->controller_choice->Append("Sustenuto (67)");
	this->controller_choice->Append("Soft pedal (68)");
	this->controller_choice->Append("Resonance (71)");
	this->controller_choice->Append("Cutoff (72)");
	this->controller_choice->Append("Release (73)");
	this->controller_choice->Append("Attack (74)");
	this->controller_choice->Append("Portamento amount (85)");
	this->controller_choice->Append("Reverb (92)");
	this->controller_choice->Append("Tremolo (93)");
	this->controller_choice->Append("Chorus (94)");
	this->controller_choice->Append("Detune (95)");
	this->controller_choice->Append("Phaser (96)");
	for (int i = 1; i <= 128; i++) this->controller_choice->Append(wxString::Format("%d", i));
	properties_sizer->Add(this->controller_choice, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));

	properties_sizer->AddSpacer(0);
	this->start_lane_group_check_box = new wxCheckBox(this, wxID_ANY, "Start lane group");
	properties_sizer->Add(this->start_lane_group_check_box, wxSizerFlags().Expand().Border());

	wxSizer* button_sizer = this->CreateButtonSizer(wxOK | wxCANCEL);
	outer_sizer->Add(button_sizer, wxSizerFlags().Align(wxALIGN_CENTER));

	outer_sizer->Fit(this);
	this->CenterOnParent();

	wxString lane_type = this->lane_type_choice->GetString(this->lane_type_choice->GetSelection());
	this->controller_choice->Enable(lane_type.IsSameAs("Controller lane"));

	this->lane_type_choice->Bind(wxEVT_CHOICE, [=](wxCommandEvent& event) { this->OnLaneTypeChoice(event); });
	this->Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) { this->OnClose(event); });
}

LanePropertiesDialog::~LanePropertiesDialog()
{
}

void LanePropertiesDialog::OnLaneTypeChoice(wxCommandEvent& event)
{
	wxString lane_type = event.GetString();
	this->controller_choice->Enable(lane_type.IsSameAs("Controller lane"));
	event.Skip();
}

void LanePropertiesDialog::OnClose(wxCloseEvent& event)
{
	if (this->GetReturnCode() == wxID_OK)
	{
	}

	event.Skip();
}


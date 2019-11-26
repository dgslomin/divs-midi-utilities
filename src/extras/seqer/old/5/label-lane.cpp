
#include <wx/wx.h>

LabelLane::LabelLane(Window* window): wxWindow(window, wxID_ANY)
{
	this->Bind(wxEVT_PAINT, &OnPaint);
}

LabelLane::~LabelLane()
{
}

void LabelLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	this->PopulateLabels();
	this->LayoutLabels();

	for (Label* label: this->labels)
	{
		dc.DrawText(label->x, label->row * this->row_height, label->text);
	}
}

void LabelLane::LayoutLabels(void)
{
	for (Label* label: this->labels)
	{
		label->x = label->preferred_x;
		label->row = 0;
	}
}


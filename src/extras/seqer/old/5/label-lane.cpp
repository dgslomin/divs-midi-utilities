
#include <wx/wx.h>
#include "label-lane.h"
#include "lane.h"
#include "window.h"

LabelLane::LabelLane(Window* window): Lane(window)
{
}

LabelLane::~LabelLane()
{
}

void LabelLane::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	int row_height = dc.GetCharHeight();

	this->PopulateLabels();
	this->LayoutLabels();

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		dc.DrawText(this->labels[label_number].text, this->labels[label_number].x, this->labels[label_number].row * row_height);
	}
}

void LabelLane::PopulateLabels()
{
}

void LabelLane::LayoutLabels()
{
	wxClientDC dc(this);
	std::vector<int> row_ends;

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		this->labels[label_number].x = this->window->GetXFromTick(MidiFileEvent_getTick(this->labels[label_number].midi_event));
		this->labels[label_number].width = dc.GetTextExtent(this->labels[label_number].text).GetWidth();

		for (this->labels[label_number].row = 0; this->labels[label_number].row < row_ends.size(); this->labels[label_number].row++)
		{
			if (this->labels[label_number].x >= row_ends[this->labels[label_number].row]) break;
		}

		int row_end = this->labels[label_number].x + this->labels[label_number].width;

		if (this->labels[label_number].row < row_ends.size())
		{
			row_ends[this->labels[label_number].row] = row_end;
		}
		else
		{
			row_ends.push_back(row_end);
		}
	}
}


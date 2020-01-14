
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
	int row_height = dc.GetCharHeight() + 4;

	this->PopulateLabels();
	this->LayoutLabels();

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		dc.DrawText(label.text, label.x + 2, (label.row * row_height) + 2);
	}
}

void LabelLane::LayoutLabels()
{
	wxClientDC dc(this);
	std::vector<int> row_ends;

	for (int label_number = 0; label_number < this->labels.size(); label_number++)
	{
		Label& label = this->labels[label_number];
		label.x = this->window->GetXFromTick(MidiFileEvent_getTick(label.midi_event));
		label.width = dc.GetTextExtent(label.text).GetWidth() + 4;

		for (label.row = 0; label.row < row_ends.size(); label.row++)
		{
			if (label.x >= row_ends[label.row]) break;
		}

		int row_end = label.x + label.width;

		if (label.row < row_ends.size())
		{
			row_ends[label.row] = row_end;
		}
		else
		{
			row_ends.push_back(row_end);
		}
	}
}

MidiFileEvent_t LabelLane::GetEventFromXY(int x, int y)
{
	wxPaintDC dc(this);
	int row_height = dc.GetCharHeight() + 4;

	for (int label_number = this->labels.size() - 1; label_number >= 0; label_number--)
	{
		Label& label = this->labels[label_number];
		wxRect rect = wxRect(label.x, label.row * row_height, label.width, row_height);
		if (rect.Contains(x, y)) return label.midi_event;
	}

	return NULL;
}


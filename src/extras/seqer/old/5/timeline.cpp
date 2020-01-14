
#include <wx/wx.h>
#include "timeline.h"
#include "window.h"

Timeline::Timeline(Window* window): wxWindow(window, wxID_ANY)
{
	this->window = window;
	this->Bind(wxEVT_PAINT, [=](wxPaintEvent& event) { this->OnPaint(event); });
}

Timeline::~Timeline()
{
}

void Timeline::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	long min_tick = this->window->GetTickFromX(0);
	long max_tick = this->window->GetTickFromX(this->GetSize().GetWidth());

	this->PaintMeasureBeatTimeline(dc, min_tick, max_tick, 0);
	this->PaintHourMinuteSecondTimeline(dc, min_tick, max_tick, dc.GetCharHeight());
}

void Timeline::PaintMeasureBeatTimeline(wxDC& dc, long min_tick, long max_tick, int y)
{
	int min_beat = (int)(MidiFile_getBeatFromTick(this->window->sequence->midi_file, min_tick));
	int max_beat = (int)(MidiFile_getBeatFromTick(this->window->sequence->midi_file, max_tick));
	int max_x = 0;

	for (int beat = min_beat; beat < max_beat; beat++)
	{
		long tick = MidiFile_getTickFromBeat(this->window->sequence->midi_file, beat);
		wxString label = wxString(MidiFile_getMeasureBeatStringFromTick(this->window->sequence->midi_file, tick));
		int x = this->window->GetXFromTick(tick);

		if (x >= max_x)
		{
			dc.DrawText(label, x, y);
			max_x = x + dc.GetTextExtent(label).GetWidth();
		}
	}
}

void Timeline::PaintHourMinuteSecondTimeline(wxDC& dc, long min_tick, long max_tick, int y)
{
	int min_time = (int)(MidiFile_getTimeFromTick(this->window->sequence->midi_file, min_tick));
	int max_time = (int)(MidiFile_getTimeFromTick(this->window->sequence->midi_file, max_tick));
	int max_x = 0;

	for (int time = min_time; time < max_time; time++)
	{
		long tick = MidiFile_getTickFromTime(this->window->sequence->midi_file, time);
		wxString label = wxString(MidiFile_getMeasureBeatStringFromTick(this->window->sequence->midi_file, tick));
		int x = this->window->GetXFromTick(tick);

		if (x >= max_x)
		{
			dc.DrawText(label, x, y);
			max_x = x + dc.GetTextExtent(label).GetWidth();
		}
	}
}



#include <algorithm>
#include <wx/wx.h>
#include <midifile.h>
#include "sequence-editor.h"
#include "event-list.h"
#include "piano-roll.h"
#include "music-math.h"
#include "color.h"

EventList::EventList(SequenceEditor* sequence_editor)
{
	this->sequence_editor = sequence_editor;
	this->font = this->sequence_editor->window->application->default_event_list_font;
}

void EventList::RefreshData()
{
	wxClientDC dc(this->sequence_editor);
	dc.SetFont(this->font);
	this->row_height = dc.GetCharHeight();

	wxColour button_color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	this->current_cell_border_color = ColorShade(button_color, 25);

	this->column_widths[0] = dc.GetTextExtent("Marker#").GetWidth();
	this->column_widths[1] = dc.GetTextExtent(this->sequence_editor->step_size->GetTimeStringFromTick(0) + "###").GetWidth();
	this->column_widths[2] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[3] = dc.GetTextExtent("00#").GetWidth();
	this->column_widths[4] = dc.GetTextExtent("C#0#").GetWidth();
	this->column_widths[5] = dc.GetTextExtent("000#").GetWidth();
	this->column_widths[6] = dc.GetTextExtent(this->sequence_editor->step_size->GetTimeStringFromTick(0) + "###").GetWidth();
	this->column_widths[7] = dc.GetTextExtent("000#").GetWidth();
}

void EventList::OnDraw(wxDC& dc)
{
	long first_y = this->sequence_editor->GetFirstVisibleY();
	long last_y = this->sequence_editor->GetLastVisibleY();
	long first_step_number = this->sequence_editor->GetStepNumberFromRowNumber(this->GetRowNumberFromY(first_y));
	long last_step_number = this->sequence_editor->GetStepNumberFromRowNumber(this->GetRowNumberFromY(last_y));
	long first_row_number = this->GetFirstVisibleRowNumber();
	long last_row_number = this->GetLastVisibleRowNumber();
	long piano_roll_width = this->sequence_editor->piano_roll->GetWidth();
	long width = this->GetVisibleWidth();

	dc.SetPen(wxPen(this->sequence_editor->piano_roll->lighter_line_color));

	for (long step_number = first_step_number; step_number <= last_step_number; step_number++)
	{
		long y = this->sequence_editor->piano_roll->GetYFromStepNumber(step_number);
		if (y == 0) continue;
		dc.DrawLine(piano_roll_width, y, piano_roll_width + width, y);
	}

	dc.SetPen(this->current_cell_border_color);
	dc.DrawRectangle(this->GetXFromColumnNumber(this->sequence_editor->current_column_number) - 1, this->GetYFromRowNumber(this->sequence_editor->current_row_number), this->GetColumnWidth(this->sequence_editor->current_column_number) + 1, this->row_height + 1);

	dc.SetFont(this->font);

	for (long row_number = first_row_number; row_number <= last_row_number; row_number++)
	{
		for (long column_number = 0; column_number < 8; column_number++)
		{
			wxString cell_text = this->sequence_editor->rows[row_number]->cells[column_number]->GetValueText();
			if (!cell_text.IsEmpty()) dc.DrawText(cell_text, this->GetXFromColumnNumber(column_number) + (column_number == 0 ? 1 : 0), this->GetYFromRowNumber(row_number) + 1);
		}
	}
}

long EventList::GetVisibleWidth()
{
	return this->sequence_editor->GetVisibleWidth() - this->sequence_editor->piano_roll->GetWidth();
}

long EventList::GetFirstVisibleRowNumber()
{
	return this->sequence_editor->GetViewStart().y;
}

long EventList::GetLastVisibleRowNumber()
{
	return this->GetFirstVisibleRowNumber() + this->GetRowNumberFromY(this->sequence_editor->GetVisibleHeight()) + 1;
}

long EventList::GetLastVisiblePopulatedRowNumber()
{
	return std::min<int>(this->GetLastVisibleRowNumber(), (long)(this->sequence_editor->rows.size() - 1));
}

long EventList::GetColumnWidth(long column_number)
{
	return this->column_widths[column_number];
}

long EventList::GetXFromColumnNumber(long column_number)
{
	long x = this->sequence_editor->piano_roll->GetWidth();
	while (--column_number >= 0) x += this->column_widths[column_number];
	return x;
}

long EventList::GetYFromRowNumber(long row_number)
{
	return row_number * this->row_height;
}

long EventList::GetRowNumberFromY(long y)
{
	return y / this->row_height;
}

